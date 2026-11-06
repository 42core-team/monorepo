#!/usr/bin/env python3
"""Black-box tests for login registration (issue #244). Needs a built server binary, uses port 4444.

Usage: python3 server/tests/test_login_registration.py [RegistrationTest.test_name]
"""

import json
import os
import socket
import subprocess
import sys
import tempfile
import threading
import time
import unittest

SERVER_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SERVER_BIN = os.path.join(SERVER_ROOT, "server")
GAME_CONFIG = os.path.join(SERVER_ROOT, "..", "bots", "configs", "fake_demo", "game.config.json")
PORT = 4444

CONNECT_TIMEOUT_MS = 2000
LOGIN_TIMEOUT_MS = 500
SLACK_S = 1.5  # scheduling tolerance on top of a deadline

STALL_ACTIONS_PER_TICK = 1000
STALL_TICKS = 200

DID_NOT_CONNECT = 4
DISCONNECTED = 2


def login_line(team_id, name="bot"):
    return (json.dumps({"id": team_id, "password": "42", "name": name}) + "\n").encode()


class ServerRun:
    """Runs the server for one test, with short timeouts and a private replay folder."""

    def __init__(self, teams=(1, 2), connect_timeout_ms=CONNECT_TIMEOUT_MS, login_timeout_ms=LOGIN_TIMEOUT_MS):
        self.dir = tempfile.TemporaryDirectory()
        config = {
            "replayFolderPaths": [self.dir.name],
            "timeoutTicks": 30000,
            "timeoutMs": 3000000,
            "clientWaitTimeoutMs": 300,
            "clientConnectTimeoutMs": connect_timeout_ms,
            "clientLoginTimeoutMs": login_timeout_ms,
            "clientPacketsMaxSizeKb": 16,
        }
        config_path = os.path.join(self.dir.name, "server.config.json")
        with open(config_path, "w") as f:
            json.dump(config, f)
        self.log_path = os.path.join(self.dir.name, "server.log")
        self.log = open(self.log_path, "w")
        self.started = time.monotonic()
        self.proc = subprocess.Popen(
            [SERVER_BIN, config_path, GAME_CONFIG, os.path.join(SERVER_ROOT, "data")] + [str(t) for t in teams],
            stdout=self.log,
            stderr=subprocess.STDOUT,
        )
        self.wait_until_listening()

    def wait_until_listening(self):
        # connecting would use up a pending slot, so watch the log instead
        for _ in range(500):
            if "listening on port" in self.output():
                return
            if self.proc.poll() is not None:
                raise AssertionError("server exited early:\n" + self.output())
            time.sleep(0.01)
        raise AssertionError("server did not start listening:\n" + self.output())

    def output(self):
        self.log.flush()
        with open(self.log_path) as f:
            return f.read()

    def connect(self):
        return socket.create_connection(("127.0.0.1", PORT), timeout=5)

    def wait(self, timeout):
        """Returns how long the server ran."""
        try:
            self.proc.wait(timeout)
        except subprocess.TimeoutExpired:
            raise AssertionError("server still running after %.1fs:\n%s" % (timeout, self.output()))
        return time.monotonic() - self.started

    def results(self):
        with open(os.path.join(self.dir.name, "replay_latest.json")) as f:
            return {r["id"]: r for r in json.load(f)["misc"]["team_results"]}

    def close(self):
        if self.proc.poll() is None:
            self.proc.kill()
            self.proc.wait()
        self.log.close()
        self.dir.cleanup()


class HealthyBot:
    """Logs in and reads whatever the server sends until it closes the connection."""

    def __init__(self, run, team_id, name="bot"):
        self.sock = run.connect()
        self.sock.sendall(login_line(team_id, name))
        self.received = b""
        self.thread = threading.Thread(target=self.read_all, daemon=True)
        self.thread.start()

    def read_all(self):
        try:
            while True:
                data = self.sock.recv(65536)
                if not data:
                    return
                self.received += data
        except OSError:
            pass

    def got_config(self):
        return b"\n" in self.received

    def close(self):
        self.sock.close()


def wait_closed(sock, timeout):
    """Returns the seconds until the server closed the connection, or None if it stayed open."""
    start = time.monotonic()
    sock.settimeout(timeout)
    try:
        while True:
            if not sock.recv(65536):
                return time.monotonic() - start
    except ConnectionResetError:
        return time.monotonic() - start
    except socket.timeout:
        return None


class RegistrationTest(unittest.TestCase):
    def setUp(self):
        self.socks = []

    def tearDown(self):
        for sock in self.socks:
            sock.close()
        self.run_.close()

    def start(self, **kwargs):
        self.run_ = ServerRun(**kwargs)
        return self.run_

    def connect(self):
        sock = self.run_.connect()
        self.socks.append(sock)
        return sock

    def bot(self, team_id, name="bot"):
        bot = HealthyBot(self.run_, team_id, name)
        self.socks.append(bot.sock)
        return bot

    def assert_started_early(self, elapsed):
        self.assertLess(elapsed, CONNECT_TIMEOUT_MS / 1000 - 0.2, "healthy registration waited for the deadline")

    def test_silent_peers_do_not_block_healthy_clients(self):
        run = self.start(connect_timeout_ms=30000, login_timeout_ms=20000)
        for _ in range(3):
            self.connect()
        bots = [self.bot(1), self.bot(2)]
        for _ in range(100):
            if "with 2 / 2 teams" in run.output():
                break
            time.sleep(0.02)
        self.assertIn("with 2 / 2 teams", run.output())
        for bot in bots:
            bot.close()
        run.wait(10)

    def test_trickling_peer_is_closed_at_its_absolute_deadline(self):
        self.start(connect_timeout_ms=CONNECT_TIMEOUT_MS * 3)
        sock = self.connect()
        stop = threading.Event()

        def trickle():
            while not stop.wait(0.05):
                try:
                    sock.send(b" ")
                except OSError:
                    return

        thread = threading.Thread(target=trickle, daemon=True)
        thread.start()
        closed_after = wait_closed(sock, LOGIN_TIMEOUT_MS / 1000 + SLACK_S)
        stop.set()
        thread.join()
        self.assertIsNotNone(closed_after, "traffic extended the login deadline")
        self.assertGreater(closed_after, LOGIN_TIMEOUT_MS / 1000 - 0.1)

    def test_trickling_peer_does_not_block_healthy_clients(self):
        run = self.start(connect_timeout_ms=30000, login_timeout_ms=20000)
        sock = self.connect()
        sock.send(b'{"id": 1,')
        bots = [self.bot(1), self.bot(2)]
        for _ in range(100):
            if "with 2 / 2 teams" in run.output():
                break
            time.sleep(0.02)
        self.assertIn("with 2 / 2 teams", run.output())
        for bot in bots:
            bot.close()
        run.wait(10)

    def test_reconnects_do_not_extend_startup(self):
        run = self.start()
        stop = threading.Event()

        def reconnect_forever():
            while not stop.is_set():
                try:
                    sock = socket.create_connection(("127.0.0.1", PORT), timeout=1)
                    sock.send(b"{")
                    time.sleep(0.02)
                    sock.close()
                except OSError:
                    return

        thread = threading.Thread(target=reconnect_forever, daemon=True)
        thread.start()
        elapsed = run.wait(CONNECT_TIMEOUT_MS / 1000 + SLACK_S)
        stop.set()
        thread.join()
        self.assertGreater(elapsed, CONNECT_TIMEOUT_MS / 1000 - 0.1)

    def test_more_peers_than_pending_slots_cannot_lock_out_a_client(self):
        run = self.start(connect_timeout_ms=30000, login_timeout_ms=20000)
        for _ in range(100):
            self.connect()
        bots = [self.bot(1), self.bot(2)]
        for _ in range(250):
            if "with 2 / 2 teams" in run.output():
                break
            time.sleep(0.02)
        self.assertIn("with 2 / 2 teams", run.output())
        # the oldest idle connections were dropped to stay within the bound
        self.assertIn("Too many pending logins", run.output())
        for bot in bots:
            bot.close()
        run.wait(10)

    def test_oversized_login_is_closed_promptly(self):
        self.start(login_timeout_ms=20000)
        sock = self.connect()
        try:
            sock.sendall(b"x" * 8192)
        except OSError:
            pass
        self.assertIsNotNone(wait_closed(sock, 1), "oversized login was buffered instead of rejected")

    def test_invalid_logins_are_closed_and_assign_no_state(self):
        run = self.start()
        for line in (
            b"not json\n",
            b'{"id": 1, "password": "wrong", "name": "stolen"}\n',
            login_line(99, "unexpected"),
        ):
            sock = self.connect()
            sock.sendall(line)
            self.assertIsNotNone(wait_closed(sock, 1), line)
        run.wait(CONNECT_TIMEOUT_MS / 1000 + SLACK_S)
        results = run.results()
        self.assertEqual(sorted(results), [1, 2], "an unexpected team was registered")
        for team_id, result in results.items():
            self.assertEqual(result["name"], "Team%d" % team_id)
            self.assertEqual(result["death_reason"], DID_NOT_CONNECT)

    def test_duplicate_login_is_closed_and_keeps_first_client(self):
        run = self.start()
        first = self.bot(1, "first")
        for _ in range(100):
            if "Accepted team 1" in run.output():
                break
            time.sleep(0.02)
        duplicate = self.connect()
        duplicate.sendall(login_line(1, "second"))
        self.assertIsNotNone(wait_closed(duplicate, 1))
        run.wait(CONNECT_TIMEOUT_MS / 1000 + SLACK_S)
        self.assertEqual(run.results()[1]["name"], "first")
        first.close()

    def test_login_split_across_packets_is_accepted(self):
        run = self.start(login_timeout_ms=1000)
        line = login_line(1, "split")
        sock = self.connect()
        for byte in (line[:10], line[10:]):
            sock.sendall(byte)
            time.sleep(0.1)
        run.wait(CONNECT_TIMEOUT_MS / 1000 + SLACK_S)
        self.assertEqual(run.results()[1]["name"], "split")

    def test_data_sent_right_after_login_reaches_the_game(self):
        run = self.start()
        sock = self.connect()
        sock.sendall(login_line(1) + b'{"not_a_client_packet": true}\n')
        other = self.bot(2)
        for _ in range(100):
            if "with 2 / 2 teams" in run.output():
                break
            time.sleep(0.02)
        self.assertIn("with 2 / 2 teams", run.output())
        for _ in range(100):
            if "Invalid client message schema from team 1" in run.output():
                break
            time.sleep(0.02)
        self.assertIn("Invalid client message schema from team 1", run.output())
        other.close()
        sock.close()
        run.wait(10)

    def test_zero_players(self):
        run = self.start()
        elapsed = run.wait(CONNECT_TIMEOUT_MS / 1000 + SLACK_S)
        self.assertEqual(run.proc.returncode, 0, run.output())
        self.assertGreater(elapsed, CONNECT_TIMEOUT_MS / 1000 - 0.1)
        results = run.results()
        self.assertEqual(sorted(r["place"] for r in results.values()), [0, 1])
        for result in results.values():
            self.assertEqual(result["death_reason"], DID_NOT_CONNECT)

    def test_one_player_wins_by_default(self):
        run = self.start()
        bot = self.bot(2, "solo")
        run.wait(CONNECT_TIMEOUT_MS / 1000 + SLACK_S)
        self.assertEqual(run.proc.returncode, 0, run.output())
        results = run.results()
        self.assertEqual(results[1]["death_reason"], DID_NOT_CONNECT)
        self.assertEqual(results[1]["place"], 1)
        self.assertNotEqual(results[2]["death_reason"], DID_NOT_CONNECT)
        self.assertEqual(results[2]["place"], 0)
        self.assertEqual(results[2]["name"], "solo")
        bot.thread.join(2)
        self.assertTrue(bot.got_config(), "the final message was dropped on shutdown")

    def test_client_leaving_before_start_is_disconnected_without_a_core(self):
        run = self.start()
        sock = self.connect()
        sock.sendall(login_line(1, "leaver"))
        for _ in range(100):
            if "Accepted team 1" in run.output():
                break
            time.sleep(0.02)
        sock.close()
        survivor = self.bot(2, "survivor")
        run.wait(5)
        self.assertEqual(run.proc.returncode, 0, run.output())
        results = run.results()
        self.assertEqual(results[1]["death_reason"], DISCONNECTED)
        self.assertEqual(results[1]["place"], 1)
        self.assertEqual(results[2]["place"], 0)
        self.assertIn("with 1 / 2 teams", run.output())
        survivor.close()

    @unittest.skipUnless(sys.platform == "linux", "needs POLLRDHUP")
    def test_client_leaving_behind_unread_data_is_disconnected_without_a_core(self):
        run = self.start()
        sock = self.connect()
        sock.sendall(login_line(1, "leaver") + b'{"actions": []}\n')
        for _ in range(100):
            if "Accepted team 1" in run.output():
                break
            time.sleep(0.02)
        sock.close()
        survivor = self.bot(2, "survivor")
        run.wait(5)
        self.assertIn("with 1 / 2 teams", run.output())
        self.assertEqual(run.results()[1]["death_reason"], DISCONNECTED)
        survivor.close()

    def test_blocked_writer_does_not_hang_shutdown(self):
        run = self.start()
        # shrink the window before connecting, then never read, so the server's sends to this client block
        stalled = socket.socket()
        self.socks.append(stalled)
        stalled.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 1024)
        stalled.connect(("127.0.0.1", PORT))
        stalled.sendall(login_line(1, "stalled"))
        healthy = self.connect()
        healthy.sendall(login_line(2, "healthy"))

        # invalid actions are echoed as errors to their sender, so each tick queues far more for the stalled client
        flood = json.dumps({"actions": [{"type": "x"}] * STALL_ACTIONS_PER_TICK}).encode() + b"\n"
        actions = b'{"actions": []}\n'
        healthy.settimeout(10)
        ticks = 0
        while ticks < STALL_TICKS:
            data = healthy.recv(65536)
            self.assertTrue(data, run.output())
            for _ in range(data.count(b"\n")):
                stalled.sendall(flood)
                healthy.sendall(actions)
                ticks += 1

        healthy.close()  # ends the game, which destroys the stalled client's Bridge with its writer blocked
        run.wait(5)
        self.assertEqual(run.proc.returncode, 0, run.output())


if __name__ == "__main__":
    unittest.main(verbosity=2)

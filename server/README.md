# CORE Game Server

## Running

```sh
./server <server config> <game config> <data folder> <teamId1> <teamId2> ...
```

Bots connect over TCP and log in with a JSON line `{"id": <teamId>, "password": "42", "name": "..."}`.
Without `TEAM_PORTS` the server listens on port 4444 and accepts any expected team ID there, first login per ID wins.

## Environment

All variables are optional.

| Variable               | Read by          | Meaning                                                                         |
| ---------------------- | ---------------- | ------------------------------------------------------------------------------- |
| `GAME_ID`              | server           | Written into the replay; its last hex digit decides which team gets which core. |
| `PLAYER_<teamId>_NAME` | server           | Team name for the replay, takes precedence over the name the bot sends.         |
| `TEAM_PORTS`           | server           | Per-team login ports, see below.                                                |
| `UPLOAD_REPLAY`        | image entrypoint | `true` uploads the replay to S3 after the game.                                 |
| `SEND_RESULTS`         | image entrypoint | `true` sends the results to RabbitMQ after the game.                            |
| `REPLAY_FILE`          | image entrypoint | Replay the entrypoint uploads or sends, default `./replays/replay_latest.json`. |

### `TEAM_PORTS`

Comma-separated `<teamId>=<port>` pairs without spaces, for example `TEAM_PORTS=123456=4501,654321=4502`.

- Unset or empty: one listener on port 4444 for all teams.
- Set: the server listens only on the listed ports (not on 4444) and accepts a login only if its team ID is the one
  mapped to the port the connection arrived on. Other logins are closed and logged as a warning.
- The value must list every team ID passed on the command line exactly once, no other IDs, and distinct ports in
  1..65535. Anything else stops the server at startup; it never falls back to the shared port.
- If any port, team or shared, cannot be opened (for example because another process already listens on it), the
  server exits at startup.

This is only a security boundary if the orchestrator runs each bot under its own user and its firewall redirects that
bot's connections to port 4444 to its team's port and blocks the other teams' ports. Without that firewall any local
process can still connect to any team's port.

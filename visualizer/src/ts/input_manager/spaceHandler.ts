import {
	getPlaybackSpeed,
	isPlaying,
	maxSpeed,
	pausePlayback,
	setPlaybackSpeed,
	startPlayback,
} from "./timeManager";

const playButton = document.getElementById(
	"play-pause-button",
) as HTMLButtonElement;

export function initDoubleSpeedHandler() {
	const HOLD_THRESHOLD_MS = 150;
	const BOOST_MULTIPLIER = 2.5;

	let spaceDown = false;
	let holdTimer: number | null = null;
	let boostActive = false;

	let startedPausedForHold = false;

	function onKeyDown(e: KeyboardEvent) {
		if (e.code !== "Space") return;

		e.preventDefault();

		playButton.classList.add("active");

		if (spaceDown) return;
		spaceDown = true;

		// start up boost
		holdTimer = window.setTimeout(() => {
			holdTimer = null;

			boostActive = true;

			if (!isPlaying()) {
				startedPausedForHold = true;
				startPlayback();
			}

			setPlaybackSpeed(
				Math.min(maxSpeed, getPlaybackSpeed() * BOOST_MULTIPLIER),
			);
		}, HOLD_THRESHOLD_MS);
	}

	function onKeyUp(e: KeyboardEvent) {
		if (e.code !== "Space") return;
		e.preventDefault();

		playButton.classList.remove("active");

		if (holdTimer !== null) {
			window.clearTimeout(holdTimer);
			holdTimer = null;

			if (isPlaying()) {
				pausePlayback();
			} else {
				startPlayback();
			}
		} else {
			if (boostActive) {
				boostActive = false;

				setPlaybackSpeed(getPlaybackSpeed() / BOOST_MULTIPLIER);

				if (startedPausedForHold && isPlaying()) {
					pausePlayback();
				}
			}
		}

		spaceDown = false;
		startedPausedForHold = false;
	}

	function forceCancelBoostIfActive() {
		if (holdTimer !== null) {
			window.clearTimeout(holdTimer);
			holdTimer = null;
		}
		if (boostActive) {
			boostActive = false;
			setPlaybackSpeed(getPlaybackSpeed() / BOOST_MULTIPLIER);
			if (startedPausedForHold && isPlaying()) {
				pausePlayback();
			}
		}
		spaceDown = false;
		startedPausedForHold = false;
	}

	window.addEventListener("blur", forceCancelBoostIfActive);
	document.addEventListener("visibilitychange", () => {
		if (document.visibilityState !== "visible") forceCancelBoostIfActive();
	});
	window.addEventListener("pagehide", forceCancelBoostIfActive);

	window.addEventListener("keydown", onKeyDown, { capture: true });
	window.addEventListener("keyup", onKeyUp, { capture: true });
}

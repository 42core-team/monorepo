export function setupInfoPopupManager() {
	const dialog = document.getElementById("site-modal");
	const openBtn = document.getElementById("open-modal");
	const closeBtn = document.getElementById("close-modal");

	console.log('dialog', dialog);
	console.log('openBtn', openBtn);
	console.log('closeBtn', closeBtn);

	if (dialog instanceof HTMLDialogElement) {
		openBtn?.addEventListener("click", () => dialog.showModal());
		closeBtn?.addEventListener("click", () => dialog.close());
		dialog.addEventListener("click", (e) => {
			const r = dialog.getBoundingClientRect();
			const inDialog =
				e.clientX >= r.left &&
				e.clientX <= r.right &&
				e.clientY >= r.top &&
				e.clientY <= r.bottom;
			if (!inDialog) dialog.close();
		});
	} else {
		console.error("Dialog element not found or not supported.");
	}
}

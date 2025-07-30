const svgCanvas = document.getElementById('svg-canvas') as HTMLElement;

function updateSvgSize() {
	const svgHeight = window.innerHeight - svgCanvas.getBoundingClientRect().top;
	document.documentElement.style.setProperty('--svg-canvas-scale', `${svgHeight - 24}px`);
}
window.addEventListener('resize', updateSvgSize);
window.addEventListener('load', updateSvgSize);

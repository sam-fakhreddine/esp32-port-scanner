// Initialize Reveal.js with 16:10 aspect ratio
Reveal.initialize({
    hash: true,
    controls: true,
    progress: true,
    center: true,
    transition: 'slide',
    slideNumber: 'c/t',
    width: 1920,
    height: 1200,
    margin: 0.04,
    minScale: 0.2,
    maxScale: 2.0
});

// Scale control functionality
const scaleSlider = document.getElementById('scaleSlider');
const scaleValue = document.getElementById('scaleValue');

scaleSlider.addEventListener('input', function() {
    const scale = this.value / 100;
    scaleValue.textContent = this.value + '%';
    const margin = (1 - scale) / 2;
    Reveal.configure({ margin: margin, maxScale: scale });
});

// Initialize with default value
scaleSlider.dispatchEvent(new Event('input'));
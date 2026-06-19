// Inside your HTML script block after loading your compiled WASM module:
Module.onRuntimeInitialized = () => {
    // Initialize C++ engine grid data
    Module.initEngine(45); 

    function renderVoxelEngine() {
        ctx.clearRect(0, 0, width, height);
        
        let t = parseInt(shiftSlider.value);
        let angle = parseInt(rotSlider.value);

        // Fetch the computed and sorted 3D vectors straight from C++ memory
        let frameNodes = Module.computeFrame(t, angle, width, height, 6);

        // Loop through the vector elements built by C++
        for (let i = 0; i < frameNodes.size(); i++) {
            let node = frameNodes.get(i);
            ctx.fillStyle = `rgb(${node.r}, ${node.g}, ${node.b})`;
            ctx.fillRect(node.screenX, node.screenY, node.size, node.size * 2);
        }

        requestAnimationFrame(renderVoxelEngine);
    }
    renderVoxelEngine();
};
<script lang="ts">
    import { onMount } from "svelte";

    let canvas: HTMLCanvasElement;
    let ctx: CanvasRenderingContext2D | null;
    let eyeOffsetX = 0;
    let eyeOffsetY = 0;
    let blinkState = 0; // 0: open, 1: closing, 2: closed, 3: opening
    let blinkTimer = 0;
    let expression = "neutral"; // neutral, happy, surprised, sleepy

    // Dragging Logic
    let container: HTMLDivElement;
    let isDragging = false;
    let position = { x: 0, y: 0 };
    let dragStartOffset = { x: 0, y: 0 };
    let isMobile = false;
    let hasInitialized = false;

    function drawEye(x: number, y: number, size: number) {
        if (!ctx) return;

        ctx.fillStyle = "#FFFFFF"; // White eyes
        ctx.beginPath();

        // Make eyes larger
        const eyeScale = 1.5;
        const s = size * eyeScale;

        let height = s;
        if (blinkState !== 0) {
            if (blinkState === 2)
                height = 4; // Closed thick line
            else height = s * 0.5; // Half open
        }

        if (expression === "happy") {
            // Happy inverted arch
            ctx.lineWidth = 6;
            ctx.strokeStyle = "#FFFFFF";
            ctx.beginPath();
            ctx.arc(x, y + 5, s / 2, Math.PI, 0);
            ctx.stroke();
        } else {
            // Square eye with slight rounding
            ctx.roundRect(x - s / 2, y - height / 2, s, height, 4);
            ctx.fill();
        }
    }

    function drawEyebrow(x: number, y: number, size: number, angle: number) {
        if (!ctx) return;
        ctx.save();
        ctx.translate(x, y);
        ctx.rotate(angle);
        ctx.fillStyle = "#FFFFFF"; // White eyebrows
        ctx.fillRect(-size / 2, -size / 10, size, size / 5);
        ctx.fill();
        ctx.restore();
    }

    function drawCase() {
        if (!ctx || !canvas) return;

        const w = canvas.width;
        const h = canvas.height;

        // Get colors from CSS variables
        const styles = getComputedStyle(document.documentElement);
        // Default to orange if variable is not set (failsafe)
        const caseColor =
            styles.getPropertyValue("--deskpet-case").trim() || "#FF9F1C";

        // Flat Case Color
        ctx.fillStyle = caseColor;

        // Main Case Body (Simple Rounded Rect)
        ctx.beginPath();
        ctx.roundRect(10, 25, w - 20, h - 35, 30);
        ctx.fill();

        // Screen (Simple Rounded Rect, no bezel shading)
        const bezel = 18;
        ctx.beginPath();
        ctx.roundRect(
            10 + bezel,
            25 + bezel,
            w - 20 - bezel * 2,
            h - 35 - bezel * 2,
            15,
        );
        ctx.fillStyle = "#000000";
        ctx.fill();
    }

    function drawFace() {
        if (!ctx || !canvas) return;

        ctx.clearRect(0, 0, canvas.width, canvas.height);

        drawCase();

        // Screen area for clipping eyes
        const bezel = 18;
        ctx.save();
        ctx.beginPath();
        ctx.roundRect(
            10 + bezel,
            25 + bezel,
            canvas.width - 20 - bezel * 2,
            canvas.height - 35 - bezel * 2,
            15,
        );
        ctx.clip();

        // Screen inner glow (subtle, flat)
        ctx.shadowBlur = 0;

        // Draw eyes
        const centerX = canvas.width / 2;
        const centerY = canvas.height / 2 + 10;
        const eyeSpacing = 35;
        const eyeSize = 22;

        const currentEyeX = centerX + eyeOffsetX;
        const currentEyeY = centerY + eyeOffsetY;

        drawEye(currentEyeX - eyeSpacing, currentEyeY, eyeSize);
        drawEye(currentEyeX + eyeSpacing, currentEyeY, eyeSize);

        // Draw eyebrows
        if (expression === "neutral") {
            drawEyebrow(currentEyeX - eyeSpacing, currentEyeY - 25, 20, 0);
            drawEyebrow(currentEyeX + eyeSpacing, currentEyeY - 25, 20, 0);
        }

        ctx.restore();
    }

    function update() {
        // Random blinking
        blinkTimer++;
        if (blinkState === 0 && blinkTimer > 150 + Math.random() * 200) {
            blinkState = 1;
        } else if (blinkState === 1) {
            blinkState = 2;
        } else if (blinkState === 2) {
            blinkState = 3;
        } else if (blinkState === 3) {
            blinkState = 0;
            blinkTimer = 0;
        }

        drawFace();
        requestAnimationFrame(update);
    }

    function updateEyePosition(e: MouseEvent) {
        if (!canvas) return;
        const rect = canvas.getBoundingClientRect();
        const centerX = rect.left + rect.width / 2;
        const centerY = rect.top + rect.height / 2;

        // Calculate look direction bounded
        const maxOffset = 10;
        const dx = (e.clientX - centerX) / (window.innerWidth / 2);
        const dy = (e.clientY - centerY) / (window.innerHeight / 2);

        eyeOffsetX = dx * maxOffset;
        eyeOffsetY = dy * maxOffset;
    }

    function handleMouseDown(e: MouseEvent) {
        if (isMobile) return;
        isDragging = true;
        const rect = container.getBoundingClientRect();
        dragStartOffset = {
            x: e.clientX - rect.left,
            y: e.clientY - rect.top,
        };
    }

    function handleWindowMouseMove(e: MouseEvent) {
        updateEyePosition(e);

        if (isDragging) {
            position = {
                x: e.clientX - dragStartOffset.x,
                y: e.clientY - dragStartOffset.y,
            };
        }
    }

    function handleWindowMouseUp() {
        if (!isDragging) return;
        isDragging = false;
        snapToEdge();
    }

    function snapToEdge() {
        if (!container) return;
        const w = window.innerWidth;
        const h = window.innerHeight;
        const cw = container.offsetWidth;
        const ch = container.offsetHeight;
        const m = 32; // margin 32px

        // Define 8 points
        const points = [
            { x: m, y: m }, // TL
            { x: (w - cw) / 2, y: m }, // TC
            { x: w - cw - m, y: m }, // TR
            { x: m, y: (h - ch) / 2 }, // ML
            { x: w - cw - m, y: (h - ch) / 2 }, // MR
            { x: m, y: h - ch - m }, // BL
            { x: (w - cw) / 2, y: h - ch - m }, // BC
            { x: w - cw - m, y: h - ch - m }, // BR (Default)
        ];

        // Find closest
        let closest = points[7]; // Default to BR
        let minDist = Infinity;

        for (const p of points) {
            const d =
                Math.pow(position.x - p.x, 2) + Math.pow(position.y - p.y, 2);
            if (d < minDist) {
                minDist = d;
                closest = p;
            }
        }

        position = closest;
    }

    function handleResize() {
        const wasMobile = isMobile;
        isMobile = window.innerWidth < 768;

        if (!isMobile) {
            // Re-snap on resize to ensure it stays on screen
            snapToEdge();
            // Ensure hasInitialized is true if we switch to desktop
            if (wasMobile) {
                hasInitialized = true;
            }
        }
    }

    onMount(() => {
        ctx = canvas.getContext("2d");
        // Set fixed size for the "device" face
        canvas.width = 240; // Wider for ears/case
        canvas.height = 200; // Taller for ears

        isMobile = window.innerWidth < 768;

        // Initialize position at bottom right for desktop
        if (!isMobile) {
            setTimeout(() => {
                if (container) {
                    const rect = container.getBoundingClientRect();
                    position = { x: rect.left, y: rect.top };
                    hasInitialized = true;
                }
            }, 50);
        }

        window.addEventListener("mousemove", handleWindowMouseMove);
        window.addEventListener("mouseup", handleWindowMouseUp);
        window.addEventListener("resize", handleResize);
        const loop = requestAnimationFrame(update);

        return () => {
            window.removeEventListener("mousemove", handleWindowMouseMove);
            window.removeEventListener("mouseup", handleWindowMouseUp);
            window.removeEventListener("resize", handleResize);
            cancelAnimationFrame(loop);
        };
    });
</script>

<div
    bind:this={container}
    onmousedown={handleMouseDown}
    role="button"
    tabindex="0"
    class="fixed z-100 transition-all {isDragging
        ? 'duration-0 cursor-grabbing'
        : 'duration-300 cursor-grab'} {isMobile
        ? 'bottom-20 right-4 pointer-events-none'
        : ''} {!isMobile && !hasInitialized
        ? 'bottom-8 right-8 invisible'
        : ''}"
    style={!isMobile && hasInitialized
        ? `left: ${position.x}px; top: ${position.y}px;`
        : ""}
>
    <!-- Use inline style for cursor to override tailwind if needed, though class should work -->
    <canvas
        bind:this={canvas}
        class="w-32 h-28 sm:w-40 sm:h-32 md:w-52 md:h-40 drop-shadow-2xl"
    ></canvas>
</div>

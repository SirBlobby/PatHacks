<script lang="ts">
    import { onMount } from "svelte";
    import { theme } from "$lib/stores/theme";

    let canvas: HTMLCanvasElement;

    function drawCircuit() {
        if (!canvas) return;
        const ctx = canvas.getContext("2d");
        if (!ctx) return;

        const w = window.innerWidth;
        const h = window.innerHeight;
        // Handle high DPI displays
        const dpr = window.devicePixelRatio || 1;
        canvas.width = w * dpr;
        canvas.height = h * dpr;
        ctx.scale(dpr, dpr);
        canvas.style.width = `${w}px`;
        canvas.style.height = `${h}px`;

        // Get theme-aware color
        const styles = getComputedStyle(document.documentElement);
        const accentRaw = styles.getPropertyValue("--accent-color").trim();

        let accentColor = "rgba(100,100,100,0.1)";

        if (accentRaw) {
            // If strictly just numbers/percents like "74.39% 0.174 56.63"
            // check if it starts with number or %
            if (/^[\d.%]/.test(accentRaw) && !accentRaw.startsWith("oklch")) {
                accentColor = `oklch(${accentRaw} / 0.15)`;
            } else {
                accentColor = accentRaw;
            }
        }

        ctx.strokeStyle = accentColor;
        ctx.fillStyle = accentColor;
        ctx.lineWidth = 3;
        ctx.lineCap = "round";
        ctx.lineJoin = "round";

        // Clear
        ctx.clearRect(0, 0, w, h);

        const gridSize = 30;

        // Generate sparse circuit lines
        // We'll focus slightly more on edges by filtering density
        const numLines = Math.floor((w * h) / (gridSize * gridSize) / 8);

        for (let i = 0; i < numLines; i++) {
            // Pick a start point
            let x = Math.floor(Math.random() * (w / gridSize)) * gridSize;
            let y = Math.floor(Math.random() * (h / gridSize)) * gridSize;

            // Start node
            ctx.beginPath();
            ctx.arc(x, y, 2, 0, Math.PI * 2);
            ctx.fill();

            ctx.beginPath();
            ctx.moveTo(x, y);

            // Create a path
            let segments = Math.floor(Math.random() * 4) + 2;
            let dirX = 0;
            let dirY = 0;

            // Initial direction (random 4 ways)
            if (Math.random() < 0.5) {
                dirX = Math.random() < 0.5 ? 1 : -1;
            } else {
                dirY = Math.random() < 0.5 ? 1 : -1;
            }

            for (let j = 0; j < segments; j++) {
                let len = (Math.floor(Math.random() * 3) + 2) * gridSize;

                x += dirX * len;
                y += dirY * len;

                ctx.lineTo(x, y);

                // Turn 90 degrees
                if (dirX !== 0) {
                    dirX = 0;
                    dirY = Math.random() < 0.5 ? 1 : -1;
                } else {
                    dirY = 0;
                    dirX = Math.random() < 0.5 ? 1 : -1;
                }
            }

            ctx.stroke();

            // End node (hollow for variety)
            ctx.beginPath();
            ctx.arc(x, y, 2.5, 0, Math.PI * 2);
            ctx.fill();
            ctx.beginPath();
            ctx.arc(x, y, 1.5, 0, Math.PI * 2);
            ctx.fillStyle = getComputedStyle(document.body).backgroundColor; // Clear center
            ctx.fill();
            ctx.fillStyle = accentColor; // Reset fill
        }
    }

    onMount(() => {
        // Initial draw
        drawCircuit();

        // Redraw on resize
        let resizeTimer: number;
        const handleResize = () => {
            clearTimeout(resizeTimer);
            resizeTimer = setTimeout(drawCircuit, 200);
        };
        window.addEventListener("resize", handleResize);

        // Redraw on theme change
        const unsubscribe = theme.subscribe(() => {
            // Small delay to allow DOM update of CSS vars
            setTimeout(drawCircuit, 50);
        });

        return () => {
            window.removeEventListener("resize", handleResize);
            unsubscribe();
        };
    });
</script>

<div class="fixed inset-0 -z-10 pointer-events-none opacity-50">
    <canvas bind:this={canvas} class="w-full h-full"></canvas>
</div>

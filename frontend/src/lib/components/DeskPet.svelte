<script lang="ts">
    import { onMount } from "svelte";
    import { deskpet } from "$lib/stores/deskpet";

    let canvas: HTMLCanvasElement;
    let ctx: CanvasRenderingContext2D | null;
    let eyeOffsetX = 0;
    let eyeOffsetY = 0;
    let blinkState = 0; // 0: open, 1: closing, 2: closed, 3: opening
    let blinkTimer = 0;
    let expression = "neutral";

    // Subscribe to store
    deskpet.subscribe((state) => {
        expression = state.expression;
    });

    // Particle System
    interface Particle {
        x: number;
        y: number;
        vx: number;
        vy: number;
        life: number;
        size: number;
        type: "star" | "heart";
        color: string;
    }
    let particles: Particle[] = [];

    function createParticle(
        x: number,
        y: number,
        type: "star" | "heart" = "star",
    ) {
        const angle = Math.random() * Math.PI * 2;
        const speed = 0.5 + Math.random() * 1.5;
        const color = type === "heart" ? "#FF3366" : "#FFD700";

        particles.push({
            x,
            y,
            vx: Math.cos(angle) * speed,
            vy: Math.sin(angle) * speed - 0.5,
            life: 1.0,
            size: 5 + Math.random() * 5,
            type,
            color,
        });
    }

    function updateParticles() {
        for (let i = particles.length - 1; i >= 0; i--) {
            const p = particles[i];
            p.x += p.vx;
            p.y += p.vy;
            p.life -= 0.02;
            p.size *= 0.98;

            if (p.life <= 0) {
                particles.splice(i, 1);
            }
        }
    }

    function drawStar(
        ctx: CanvasRenderingContext2D,
        cx: number,
        cy: number,
        spikes: number,
        outerRadius: number,
        innerRadius: number,
    ) {
        let rot = (Math.PI / 2) * 3;
        let x = cx;
        let y = cy;
        let step = Math.PI / spikes;

        ctx.beginPath();
        ctx.moveTo(cx, cy - outerRadius);
        for (let i = 0; i < spikes; i++) {
            x = cx + Math.cos(rot) * outerRadius;
            y = cy + Math.sin(rot) * outerRadius;
            ctx.lineTo(x, y);
            rot += step;

            x = cx + Math.cos(rot) * innerRadius;
            y = cy + Math.sin(rot) * innerRadius;
            ctx.lineTo(x, y);
            rot += step;
        }
        ctx.lineTo(cx, cy - outerRadius);
        ctx.closePath();
        ctx.fill();
    }

    function drawParticles() {
        if (!ctx) return;
        particles.forEach((p) => {
            ctx!.save();
            ctx!.globalAlpha = p.life;
            ctx!.fillStyle = p.color;
            if (p.type === "star") {
                drawStar(ctx!, p.x, p.y, 5, p.size, p.size / 2);
            } else {
                ctx!.beginPath();
                ctx!.arc(p.x, p.y, p.size / 2, 0, Math.PI * 2);
                ctx!.fill();
            }
            ctx!.restore();
        });
    }

    // Dragging Logic
    let container: HTMLDivElement;
    let isDragging = false;
    let position = { x: 0, y: 0 };
    let dragStartOffset = { x: 0, y: 0 };
    let isMobile = false;
    let hasInitialized = false;

    function drawHeart(x: number, y: number, size: number) {
        if (!ctx) return;
        ctx.save();
        ctx.beginPath();
        const topCurveHeight = size * 0.3;
        ctx.moveTo(x, y + topCurveHeight);
        ctx.bezierCurveTo(
            x,
            y,
            x - size / 2,
            y,
            x - size / 2,
            y + topCurveHeight,
        );
        ctx.bezierCurveTo(
            x - size / 2,
            y + (size + topCurveHeight) / 2,
            x,
            y + (size + topCurveHeight) / 2,
            x,
            y + size,
        );
        ctx.bezierCurveTo(
            x,
            y + (size + topCurveHeight) / 2,
            x + size / 2,
            y + (size + topCurveHeight) / 2,
            x + size / 2,
            y + topCurveHeight,
        );
        ctx.bezierCurveTo(x + size / 2, y, x, y, x, y + topCurveHeight);
        ctx.fillStyle = "#FF3366";
        ctx.fill();
        ctx.restore();
    }

    function drawEye(x: number, y: number, size: number) {
        if (!ctx) return;

        if (expression === "love") {
            const pulse = 1 + Math.sin(Date.now() / 150) * 0.15;
            drawHeart(x, y - size / 2, size * 1.5 * pulse);
            return;
        }

        ctx.fillStyle = "#FFFFFF";
        ctx.beginPath();

        const eyeScale = 1.5;
        const s = size * eyeScale;

        let height = s;
        if (blinkState !== 0) {
            if (blinkState === 2) height = 4;
            else height = s * 0.5;
        }

        if (expression === "happy") {
            ctx.lineWidth = 6;
            ctx.strokeStyle = "#FFFFFF";
            ctx.beginPath();
            ctx.arc(x, y + 5, s / 2, Math.PI, 0);
            ctx.stroke();
        } else if (expression === "surprised") {
            ctx.arc(x, y, s / 1.8, 0, Math.PI * 2);
            ctx.fill();
        } else {
            // Simple square eye (Old Style) - No pupils/glints
            ctx.roundRect(x - s / 2, y - height / 2, s, height, 4);
            ctx.fill();
        }
    }

    function drawCheeks(x: number, y: number, spacing: number) {
        if (
            !ctx ||
            (expression !== "blush" &&
                expression !== "love" &&
                expression !== "happy")
        )
            return;

        // Simple flat blush
        ctx.fillStyle = "rgba(255, 100, 100, 0.4)";
        const size = 15;
        [x - spacing - 10, x + spacing + 10].forEach((cx) => {
            ctx!.beginPath();
            ctx!.arc(cx, y + 25, size, 0, Math.PI * 2);
            ctx!.fill();
        });
    }

    function drawEyebrow(x: number, y: number, size: number, angle: number) {
        if (!ctx) return;
        ctx.save();
        ctx.translate(x, y);
        ctx.rotate(angle);
        ctx.fillStyle = "#FFFFFF";
        ctx.fillRect(-size / 2, -size / 10, size, size / 5); // Simple rect
        ctx.fill();
        ctx.restore();
    }

    function drawCase() {
        if (!ctx || !canvas) return;

        const w = canvas.width;
        const h = canvas.height;

        const styles = getComputedStyle(document.documentElement);
        const caseColor =
            styles.getPropertyValue("--deskpet-case").trim() || "#FF9F1C";

        ctx.fillStyle = caseColor;

        ctx.beginPath();
        ctx.roundRect(10, 25, w - 20, h - 35, 30);
        ctx.fill();

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

        const centerX = canvas.width / 2;
        const centerY = canvas.height / 2 + 10;
        const eyeSpacing = 35;
        const eyeSize = 22;

        const currentEyeX = centerX + eyeOffsetX;
        const currentEyeY = centerY + eyeOffsetY;

        drawCheeks(currentEyeX, currentEyeY, eyeSpacing);

        drawEye(currentEyeX - eyeSpacing, currentEyeY, eyeSize);
        drawEye(currentEyeX + eyeSpacing, currentEyeY, eyeSize);

        if (expression === "neutral" || expression === "blush") {
            drawEyebrow(currentEyeX - eyeSpacing, currentEyeY - 25, 20, 0);
            drawEyebrow(currentEyeX + eyeSpacing, currentEyeY - 25, 20, 0);
        } else if (expression === "confused") {
            drawEyebrow(currentEyeX - eyeSpacing, currentEyeY - 30, 20, -0.2);
            drawEyebrow(currentEyeX + eyeSpacing, currentEyeY - 20, 20, 0.2);
        }

        ctx.restore();
    }

    function update() {
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

        if (expression === "love" || expression === "happy") {
            if (Math.random() < 0.08) {
                const cx = canvas.width / 2;
                const cy = canvas.height / 2;
                createParticle(
                    cx + (Math.random() - 0.5) * 120,
                    cy + (Math.random() - 0.5) * 60,
                    "star",
                );
            }
        }

        updateParticles();
        drawFace();
        drawParticles();
        requestAnimationFrame(update);
    }

    function updateEyePosition(e: MouseEvent) {
        if (!canvas) return;
        const rect = canvas.getBoundingClientRect();
        const centerX = rect.left + rect.width / 2;
        const centerY = rect.top + rect.height / 2;

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
        const m = 32;

        const points = [
            { x: m, y: m },
            { x: (w - cw) / 2, y: m },
            { x: w - cw - m, y: m },
            { x: m, y: (h - ch) / 2 },
            { x: w - cw - m, y: (h - ch) / 2 },
            { x: m, y: h - ch - m },
            { x: (w - cw) / 2, y: h - ch - m },
            { x: w - cw - m, y: h - ch - m },
        ];

        let closest = points[7];
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
            snapToEdge();
            if (wasMobile) {
                hasInitialized = true;
            }
        }
    }

    onMount(() => {
        ctx = canvas.getContext("2d");
        canvas.width = 240;
        canvas.height = 200;

        isMobile = window.innerWidth < 768;

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
    <canvas
        bind:this={canvas}
        class="w-32 h-28 sm:w-40 sm:h-32 md:w-52 md:h-40 drop-shadow-2xl"
    ></canvas>
</div>

<script lang="ts">
    import { onMount } from "svelte";
    import { Conversation } from "@elevenlabs/client";
    import Icon from "@iconify/svelte";
    import { getVoiceSignedUrl } from "$lib/api";
    import { deskpet } from "$lib/stores/deskpet";

    let { sourceId }: { sourceId: string } = $props();

    // Connection state
    let status = $state<"idle" | "connecting" | "connected" | "error">("idle");
    let errorMessage = $state("");
    let conversation = $state<Conversation | null>(null);

    // Voice mode: "listening" = waiting for user, "speaking" = agent talking
    let agentMode = $state<"listening" | "speaking">("listening");
    let isMicMuted = $state(false);

    // Transcript
    interface TranscriptEntry {
        role: "user" | "agent";
        text: string;
    }
    let transcript = $state<TranscriptEntry[]>([]);
    let transcriptContainer = $state<HTMLDivElement | null>(null);

    // Audio level visualization
    let inputVolume = $state(0);
    let outputVolume = $state(0);
    let animationFrameId = $state(0);

    function scrollTranscript() {
        setTimeout(() => {
            if (transcriptContainer) {
                transcriptContainer.scrollTop =
                    transcriptContainer.scrollHeight;
            }
        }, 0);
    }

    function pollVolumes() {
        if (conversation) {
            try {
                inputVolume = conversation.getInputVolume();
                outputVolume = conversation.getOutputVolume();
            } catch {
                inputVolume = 0;
                outputVolume = 0;
            }
        }
        animationFrameId = requestAnimationFrame(pollVolumes);
    }

    async function startConversation() {
        if (status === "connecting" || status === "connected") return;

        status = "connecting";
        errorMessage = "";
        transcript = [];

        try {
            const sessionInfo = await getVoiceSignedUrl(sourceId);

            // Request microphone permission
            await navigator.mediaDevices.getUserMedia({ audio: true });

            conversation = await Conversation.startSession({
                signedUrl: sessionInfo.signed_url,
                connectionType: "websocket",
                overrides: {
                    agent: {
                        prompt: {
                            prompt: sessionInfo.system_prompt,
                        },
                    },
                },
                onConnect: ({ conversationId }) => {
                    console.log(
                        "[VoiceChat] Connected, id:",
                        conversationId,
                    );
                    status = "connected";
                    deskpet.setExpression("happy", 2000);
                },
                onDisconnect: (details) => {
                    console.log("[VoiceChat] Disconnected:", details);
                    stopVolumePolling();

                    if (details.reason === "error") {
                        const msg =
                            "message" in details
                                ? details.message
                                : "Connection lost unexpectedly";
                        errorMessage = msg;
                        status = "error";
                        deskpet.setExpression("confused", 3000);
                    } else if (details.reason === "agent") {
                        // Agent ended the conversation normally
                        status = "idle";
                        deskpet.setExpression("neutral");
                    } else {
                        // User-initiated disconnect
                        status = "idle";
                        deskpet.setExpression("neutral");
                    }
                },
                onError: (message, context) => {
                    console.error(
                        "[VoiceChat] Error:",
                        message,
                        context,
                    );
                    errorMessage =
                        message || "Voice connection error occurred";
                    status = "error";
                    deskpet.setExpression("confused", 3000);
                    stopVolumePolling();
                },
                onStatusChange: ({ status: newStatus }) => {
                    console.log(
                        "[VoiceChat] Status change:",
                        newStatus,
                    );
                },
                onModeChange: ({ mode }) => {
                    agentMode =
                        mode === "speaking" ? "speaking" : "listening";
                    if (mode === "speaking") {
                        deskpet.setExpression("happy", 0);
                    } else {
                        deskpet.setExpression("neutral");
                    }
                },
                onMessage: ({ role, message }) => {
                    if (!message) return;
                    transcript = [
                        ...transcript,
                        {
                            role: role === "user" ? "user" : "agent",
                            text: message,
                        },
                    ];
                    scrollTranscript();
                },
            });

            // Start polling audio volumes
            pollVolumes();
        } catch (err: any) {
            console.error("[VoiceChat] Failed to start:", err);
            if (err.name === "NotAllowedError") {
                errorMessage =
                    "Microphone access denied. Please allow microphone access and try again.";
            } else if (
                err.message?.includes("ELEVENLABS") ||
                err.message?.includes("not configured")
            ) {
                errorMessage =
                    "Voice chat is not configured. Ask your admin to set up ElevenLabs API keys.";
            } else {
                errorMessage =
                    err.message || "Failed to start voice conversation";
            }
            status = "error";
            deskpet.setExpression("confused", 3000);
        }
    }

    async function endConversation() {
        if (conversation) {
            try {
                await conversation.endSession();
            } catch {
                // Ignore cleanup errors
            }
            conversation = null;
        }
        status = "idle";
        agentMode = "listening";
        isMicMuted = false;
        stopVolumePolling();
        deskpet.setExpression("neutral");
    }

    function toggleMic() {
        if (!conversation) return;
        isMicMuted = !isMicMuted;
        try {
            conversation.setMicMuted(isMicMuted);
        } catch {
            // Ignore if not supported
        }
    }

    function stopVolumePolling() {
        if (animationFrameId) {
            cancelAnimationFrame(animationFrameId);
            animationFrameId = 0;
        }
        inputVolume = 0;
        outputVolume = 0;
    }

    onMount(() => {
        return () => {
            if (conversation) {
                try {
                    conversation.endSession();
                } catch {
                    // Ignore
                }
            }
            stopVolumePolling();
        };
    });

    // Derived values for the audio ring visualization
    let activeVolume = $derived(
        agentMode === "speaking" ? outputVolume : inputVolume,
    );
    let ringScale = $derived(
        status === "connected" ? 1 + activeVolume * 0.5 : 1,
    );
    let innerRingScale = $derived(
        status === "connected" ? 1 + activeVolume * 0.25 : 1,
    );
</script>

<div class="flex flex-col items-center justify-center h-full gap-8 py-6 select-none">
    <!-- Main Voice Button with Audio Rings -->
    <div class="relative flex items-center justify-center">
        <!-- Outermost soft glow -->
        {#if status === "connected"}
            <div
                class="absolute w-52 h-52 rounded-full transition-transform duration-150 ease-out opacity-40
                    {agentMode === 'speaking'
                    ? 'bg-brand-accent/10'
                    : 'bg-blue-500/10'}"
                style="transform: scale({1 + activeVolume * 0.6})"
            ></div>
        {/if}

        <!-- Outer animated ring -->
        <div
            class="absolute w-44 h-44 rounded-full transition-transform duration-100 ease-out
                {status === 'connected'
                ? agentMode === 'speaking'
                    ? 'bg-brand-accent/10 border-2 border-brand-accent/30'
                    : 'bg-blue-500/10 border-2 border-blue-500/30'
                : 'border border-brand-text/10'}"
            style="transform: scale({ringScale})"
        ></div>

        <!-- Middle ring -->
        {#if status === "connected"}
            <div
                class="absolute w-36 h-36 rounded-full transition-transform duration-75 ease-out
                    {agentMode === 'speaking'
                    ? 'bg-brand-accent/8 border border-brand-accent/20'
                    : 'bg-blue-500/8 border border-blue-500/20'}"
                style="transform: scale({innerRingScale})"
            ></div>
        {/if}

        <!-- Main Button -->
        <button
            onclick={() =>
                status === "connected"
                    ? endConversation()
                    : startConversation()}
            disabled={status === "connecting"}
            class="relative z-10 w-28 h-28 rounded-full flex items-center justify-center shadow-xl transition-all duration-200 cursor-pointer
                {status === 'idle'
                ? 'bg-brand-accent hover:bg-brand-accent/90 hover:scale-105 text-black shadow-brand-accent/25'
                : status === 'connecting'
                  ? 'bg-yellow-500/80 text-black cursor-wait shadow-yellow-500/20'
                  : status === 'connected'
                    ? agentMode === 'speaking'
                        ? 'bg-brand-accent text-black shadow-brand-accent/30'
                        : 'bg-blue-500 text-white shadow-blue-500/30'
                    : 'bg-brand-accent hover:bg-brand-accent/90 hover:scale-105 text-black shadow-brand-accent/25'}"
        >
            {#if status === "connecting"}
                <Icon icon="mdi:loading" class="text-4xl animate-spin" />
            {:else if status === "connected"}
                <Icon icon="mdi:stop" class="text-4xl" />
            {:else}
                <Icon icon="mdi:microphone" class="text-4xl" />
            {/if}
        </button>
    </div>

    <!-- Status + Label -->
    <div class="flex flex-col items-center gap-1.5">
        <div class="flex items-center gap-2 text-sm font-medium">
            {#if status === "idle"}
                <span class="text-brand-text/40">Ready to talk</span>
            {:else if status === "connecting"}
                <span class="relative flex h-2 w-2">
                    <span
                        class="animate-ping absolute inline-flex h-full w-full rounded-full bg-yellow-400/60 opacity-75"
                    ></span>
                    <span
                        class="relative inline-flex rounded-full h-2 w-2 bg-yellow-500"
                    ></span>
                </span>
                <span class="text-yellow-400">Connecting...</span>
            {:else if status === "connected"}
                <span class="relative flex h-2 w-2">
                    <span
                        class="animate-ping absolute inline-flex h-full w-full rounded-full opacity-75
                            {agentMode === 'speaking' ? 'bg-brand-accent/50' : 'bg-blue-400/50'}"
                    ></span>
                    <span
                        class="relative inline-flex rounded-full h-2 w-2
                            {agentMode === 'speaking' ? 'bg-brand-accent' : 'bg-blue-500'}"
                    ></span>
                </span>
                <span class="{agentMode === 'speaking' ? 'text-brand-accent' : 'text-blue-400'}">
                    {agentMode === "speaking"
                        ? "AI is speaking..."
                        : "Listening..."}
                </span>
            {:else if status === "error"}
                <span class="flex h-2 w-2">
                    <span class="inline-flex rounded-full h-2 w-2 bg-red-500"></span>
                </span>
                <span class="text-red-400">Disconnected</span>
            {/if}
        </div>
        <p class="text-xs text-brand-text/30">
            {#if status === "idle" || status === "error"}
                Tap the mic to start a voice conversation
            {:else if status === "connecting"}
                Setting up voice connection...
            {:else if status === "connected"}
                Tap the button to end the conversation
            {/if}
        </p>
    </div>

    <!-- Controls (only when connected) -->
    {#if status === "connected"}
        <div class="flex items-center gap-3">
            <button
                onclick={toggleMic}
                class="flex items-center gap-2 px-4 py-2.5 rounded-xl border transition-all cursor-pointer
                    {isMicMuted
                    ? 'bg-red-500/10 border-red-500/30 text-red-400 hover:bg-red-500/20'
                    : 'bg-brand-text/5 border-brand-text/10 text-brand-text/60 hover:bg-brand-text/10'}"
            >
                <Icon
                    icon={isMicMuted
                        ? "mdi:microphone-off"
                        : "mdi:microphone"}
                    class="text-lg"
                />
                <span class="text-sm font-medium">
                    {isMicMuted ? "Unmute" : "Mute"}
                </span>
            </button>
            <button
                onclick={endConversation}
                class="flex items-center gap-2 px-4 py-2.5 rounded-xl border border-red-500/20 bg-red-500/5 text-red-400 hover:bg-red-500/15 transition-all cursor-pointer"
            >
                <Icon icon="mdi:phone-hangup" class="text-lg" />
                <span class="text-sm font-medium">End</span>
            </button>
        </div>
    {/if}

    <!-- Error Message -->
    {#if errorMessage}
        <div
            class="flex items-start gap-3 p-4 rounded-xl bg-red-500/10 border border-red-500/20 max-w-md"
        >
            <Icon
                icon="mdi:alert-circle-outline"
                class="text-red-400 text-xl shrink-0 mt-0.5"
            />
            <div>
                <p class="text-sm text-red-300">{errorMessage}</p>
                <button
                    onclick={startConversation}
                    class="text-sm text-red-400 hover:text-red-300 underline mt-2 cursor-pointer"
                >
                    Try again
                </button>
            </div>
        </div>
    {/if}

    <!-- Transcript -->
    {#if transcript.length > 0}
        <div class="w-full max-w-lg">
            <div class="flex items-center gap-2 mb-2">
                <Icon
                    icon="mdi:text-box-outline"
                    class="text-brand-text/30 text-sm"
                />
                <span class="text-xs font-medium text-brand-text/40 uppercase tracking-wider"
                    >Transcript</span
                >
            </div>
            <div
                bind:this={transcriptContainer}
                class="max-h-56 overflow-y-auto space-y-2.5 p-4 bg-brand-bg/50 rounded-xl border border-brand-text/10 scrollbar-thin scrollbar-thumb-brand-text/20 scrollbar-track-transparent"
            >
                {#each transcript as entry}
                    <div
                        class="flex {entry.role === 'user'
                            ? 'justify-end'
                            : 'justify-start'}"
                    >
                        <div class="max-w-[85%]">
                            <span
                                class="text-[10px] uppercase tracking-wider mb-0.5 block
                                    {entry.role === 'user'
                                    ? 'text-brand-accent/50 text-right'
                                    : 'text-brand-text/30 text-left'}"
                            >
                                {entry.role === "user" ? "You" : "AI"}
                            </span>
                            <div
                                class="px-3 py-2 rounded-xl text-sm
                                    {entry.role === 'user'
                                    ? 'bg-brand-accent/15 text-brand-text rounded-tr-none'
                                    : 'bg-brand-text/5 text-brand-text/80 rounded-tl-none'}"
                            >
                                {entry.text}
                            </div>
                        </div>
                    </div>
                {/each}
            </div>
        </div>
    {/if}
</div>

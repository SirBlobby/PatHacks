<script lang="ts">
    import Icon from "@iconify/svelte";
</script>

<div class="max-w-6xl mx-auto py-16 px-6">
    <div class="text-center mb-16">
        <h1 class="text-5xl md:text-6xl font-black mb-6 text-brand-text/90">
            Meet the <span class="text-brand-accent">LearningBuddy</span> Device
        </h1>
        <p class="text-2xl text-brand-text/60 max-w-2xl mx-auto">
            An ESP32-S3 powered companion that captures live lectures and
            streams them to your AI study dashboard in real-time.
        </p>
    </div>

    <div class="grid grid-cols-1 md:grid-cols-2 gap-12 items-center mb-20">
        <div
            class="bg-brand-surface/40 backdrop-blur rounded-3xl border border-brand-text/10 p-8 shadow-2xl relative overflow-hidden group hover:border-brand-accent/30 transition-colors duration-500"
        >
            <div
                class="absolute inset-0 bg-linear-to-br from-brand-accent/5 to-transparent opacity-0 group-hover:opacity-100 transition-opacity duration-500"
            ></div>
            <!-- Conceptual ASCII Design -->
            <div
                class="aspect-square flex items-center justify-center font-mono text-brand-accent/80 text-xs sm:text-sm select-none"
            >
                <pre class="leading-none">
      .----------------------.
     /  [ O ]          [ O ]  \
    |                          |
    |      "LISTENING..."      |
    |                          |
    |   .------------------.   |
    |   | KEY:  X9Y8Z7     |   |
    |   '------------------'   |
    |                          |
     \________________________/
         //              \\
        //                \\
</pre>
            </div>
            <div
                class="mt-8 text-center text-brand-text/40 text-sm font-mono tracking-widest uppercase"
            >
                XIAO ESP32-S3 Sense
            </div>
        </div>

        <div class="space-y-10">
            <div class="flex gap-5 group">
                <div
                    class="min-w-14 h-14 rounded-2xl bg-brand-surface border border-brand-text/10 flex items-center justify-center text-brand-text/70 group-hover:text-brand-accent group-hover:border-brand-accent/50 transition-all"
                >
                    <Icon icon="mdi:wifi-sync" width="28" />
                </div>
                <div>
                    <h3
                        class="text-xl font-bold text-brand-text mb-2 group-hover:text-brand-accent transition-colors"
                    >
                        Instant Provisioning
                    </h3>
                    <p class="text-brand-text/60 leading-relaxed">
                        Power on and connect to Wi-Fi. The device registers itself and displays a unique <span
                            class="bg-brand-accent/10 text-brand-accent px-1.5 py-0.5 rounded font-mono text-sm"
                            >6-CHAR KEY</span
                        > to pair with your account securely.
                    </p>
                </div>
            </div>

            <div class="flex gap-5 group">
                <div
                    class="min-w-14 h-14 rounded-2xl bg-brand-surface border border-brand-text/10 flex items-center justify-center text-brand-text/70 group-hover:text-brand-accent group-hover:border-brand-accent/50 transition-all"
                >
                    <Icon icon="mdi:microphone-settings" width="28" />
                </div>
                <div>
                    <h3
                        class="text-xl font-bold text-brand-text mb-2 group-hover:text-brand-accent transition-colors"
                    >
                        PDM Microphone Capture
                    </h3>
                    <p class="text-brand-text/60 leading-relaxed">
                        Records 16kHz, 16-bit mono audio via an onboard PDM
                        microphone. Audio is streamed as raw PCM over
                        WebSocket using Socket.IO for low-latency delivery.
                    </p>
                </div>
            </div>

            <div class="flex gap-5 group">
                <div
                    class="min-w-14 h-14 rounded-2xl bg-brand-surface border border-brand-text/10 flex items-center justify-center text-brand-text/70 group-hover:text-brand-accent group-hover:border-brand-accent/50 transition-all"
                >
                    <Icon icon="mdi:text-recognition" width="28" />
                </div>
                <div>
                    <h3
                        class="text-xl font-bold text-brand-text mb-2 group-hover:text-brand-accent transition-colors"
                    >
                        Auto-Transcription
                    </h3>
                    <p class="text-brand-text/60 leading-relaxed">
                        When recording stops, the backend assembles the PCM
                        stream into a WAV file and transcribes it with
                        faster-whisper. The transcript becomes a searchable
                        source you can chat with.
                    </p>
                </div>
            </div>

            <div class="flex gap-5 group">
                <div
                    class="min-w-14 h-14 rounded-2xl bg-brand-surface border border-brand-text/10 flex items-center justify-center text-brand-text/70 group-hover:text-brand-accent group-hover:border-brand-accent/50 transition-all"
                >
                    <Icon icon="mdi:emoticon-happy-outline" width="28" />
                </div>
                <div>
                    <h3
                        class="text-xl font-bold text-brand-text mb-2 group-hover:text-brand-accent transition-colors"
                    >
                        Alive & Reactive
                    </h3>
                    <p class="text-brand-text/60 leading-relaxed">
                        It's not just a box. It's a buddy. The device syncs with
                        your digital DeskPet, displaying emotions and status
                        updates in real-time.
                    </p>
                </div>
            </div>
        </div>
    </div>

</div>

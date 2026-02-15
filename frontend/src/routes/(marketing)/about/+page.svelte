<script lang="ts">
    import Icon from "@iconify/svelte";
</script>

<div class="max-w-7xl mx-auto py-20 px-6">
    <div class="text-center max-w-4xl mx-auto mb-20 relative">
        <div
            class="absolute -top-20 left-1/2 -translate-x-1/2 w-96 h-96 bg-brand-accent/5 rounded-full blur-[100px] pointer-events-none"
        ></div>

        <h1
            class="text-5xl md:text-7xl font-black mb-8 text-brand-text tracking-tighter relative z-10"
        >
            We are <span class="text-brand-accent">LearningBuddy</span>.
        </h1>
        <p
            class="text-xl md:text-3xl text-brand-text/50 font-light leading-relaxed max-w-2xl mx-auto relative z-10"
        >
            Building the bridge between physical lectures and digital
            intelligence.
        </p>
    </div>

    <!-- Features / Values Grid -->
    <div class="grid grid-cols-1 md:grid-cols-3 gap-8 mb-24 relative z-10">
        <div
            class="p-10 bg-brand-surface/30 backdrop-blur-md rounded-3xl border border-brand-text/5 hover:border-brand-accent/30 hover:bg-brand-surface/50 transition-all duration-300 group"
        >
            <div
                class="w-16 h-16 rounded-2xl bg-brand-accent/10 border border-brand-accent/20 flex items-center justify-center text-brand-accent mb-6 group-hover:scale-110 group-hover:bg-brand-accent/20 transition-transform shadow-lg shadow-brand-accent/5"
            >
                <Icon icon="mdi:brain" width="36" />
            </div>
            <h3
                class="text-2xl font-bold text-brand-text mb-4 group-hover:text-brand-accent transition-colors"
            >
                AI-First Design
            </h3>
            <p class="text-brand-text/60 leading-relaxed">
                Powered by Google Gemini for text chat and ElevenLabs for
                voice conversations. Ask questions by typing or speaking,
                with answers grounded in your actual lecture content.
            </p>
        </div>

        <div
            class="p-10 bg-brand-surface/30 backdrop-blur-md rounded-3xl border border-brand-text/5 hover:border-brand-accent/30 hover:bg-brand-surface/50 transition-all duration-300 group"
        >
            <div
                class="w-16 h-16 rounded-2xl bg-brand-accent/10 border border-brand-accent/20 flex items-center justify-center text-brand-accent mb-6 group-hover:scale-110 group-hover:bg-brand-accent/20 transition-transform shadow-lg shadow-brand-accent/5"
            >
                <Icon icon="mdi:console-network" width="36" />
            </div>
            <h3
                class="text-2xl font-bold text-brand-text mb-4 group-hover:text-brand-accent transition-colors"
            >
                Physical + Digital
            </h3>
            <p class="text-brand-text/60 leading-relaxed">
                Our ESP32-S3 hardware device streams live lecture audio over
                WebSocket. The backend transcribes it with Whisper AI and
                creates searchable study sources automatically.
            </p>
        </div>

        <div
            class="p-10 bg-brand-surface/30 backdrop-blur-md rounded-3xl border border-brand-text/5 hover:border-brand-accent/30 hover:bg-brand-surface/50 transition-all duration-300 group"
        >
            <div
                class="w-16 h-16 rounded-2xl bg-brand-accent/10 border border-brand-accent/20 flex items-center justify-center text-brand-accent mb-6 group-hover:scale-110 group-hover:bg-brand-accent/20 transition-transform shadow-lg shadow-brand-accent/5"
            >
                <Icon icon="mdi:account-school-outline" width="36" />
            </div>
            <h3
                class="text-2xl font-bold text-brand-text mb-4 group-hover:text-brand-accent transition-colors"
            >
                Student Focused
            </h3>
            <p class="text-brand-text/60 leading-relaxed">
                Created by students, for students. We understand the pain of
                missing notes and organizing study materials, so we solved it.
            </p>
        </div>
    </div>
</div>

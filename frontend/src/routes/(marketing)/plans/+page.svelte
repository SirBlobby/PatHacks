<script lang="ts">
    import Icon from "@iconify/svelte";
</script>

<div class="max-w-6xl mx-auto py-16 px-6">
    <div class="text-center mb-16">
        <h1 class="text-5xl md:text-6xl font-black mb-6 text-brand-text/90">
            Simple <span class="text-brand-accent">Pricing</span>
        </h1>
        <p class="text-xl text-brand-text/60 max-w-2xl mx-auto">
            Get started for free. Upgrade once, own it forever. No subscriptions, no surprises.
        </p>
    </div>

    <div class="grid md:grid-cols-2 gap-8 max-w-4xl mx-auto">
        <!-- Free Plan -->
        <div
            class="bg-brand-surface/80 backdrop-blur border-2 border-brand-text/10 rounded-3xl p-8 relative hover:border-brand-text/20 transition-colors flex flex-col"
        >
            <div class="mb-6">
                <h3 class="text-2xl font-bold text-brand-text">Free</h3>
                <p class="text-brand-text/60 mt-2">
                    Everything you need to get started with AI-powered studying.
                </p>
            </div>

            <div class="mb-8">
                <span class="text-5xl font-bold text-brand-text">$0</span>
                <span class="text-brand-text/50 ml-1">forever</span>
            </div>

            <ul class="space-y-4 mb-8 flex-1">
                <li class="flex items-start gap-3 text-brand-text/80">
                    <Icon icon="mdi:check-circle" class="text-xl text-green-500 mt-0.5 shrink-0" />
                    <span>AI text chat with your sources</span>
                </li>
                <li class="flex items-start gap-3 text-brand-text/80">
                    <Icon icon="mdi:check-circle" class="text-xl text-green-500 mt-0.5 shrink-0" />
                    <span>AI voice conversations</span>
                </li>
                <li class="flex items-start gap-3 text-brand-text/80">
                    <Icon icon="mdi:check-circle" class="text-xl text-green-500 mt-0.5 shrink-0" />
                    <span>AI-generated summaries & Q&A</span>
                </li>
                <li class="flex items-start gap-3 text-brand-text/80">
                    <Icon icon="mdi:check-circle" class="text-xl text-green-500 mt-0.5 shrink-0" />
                    <span>Upload PDFs, DOCX, and text files</span>
                </li>
                <li class="flex items-start gap-3 text-brand-text/80">
                    <Icon icon="mdi:check-circle" class="text-xl text-green-500 mt-0.5 shrink-0" />
                    <span>DeskPet companion</span>
                </li>
                <li class="flex items-start gap-3 text-brand-text/50">
                    <Icon icon="mdi:close-circle-outline" class="text-xl text-brand-text/30 mt-0.5 shrink-0" />
                    <span><strong class="text-brand-text/70">5 source</strong> storage limit</span>
                </li>
                <li class="flex items-start gap-3 text-brand-text/50">
                    <Icon icon="mdi:close-circle-outline" class="text-xl text-brand-text/30 mt-0.5 shrink-0" />
                    <span>No physical device</span>
                </li>
            </ul>

            <a
                href="/register"
                class="w-full py-3.5 rounded-xl border-2 border-brand-text/10 text-brand-text font-bold hover:bg-brand-text/5 transition-colors text-center block"
            >
                Get Started Free
            </a>
        </div>

        <!-- Pro Plan -->
        <div
            class="bg-brand-surface border-2 border-brand-accent rounded-3xl p-8 relative shadow-2xl shadow-brand-accent/10 flex flex-col transform md:-translate-y-4"
        >
            <div
                class="absolute -top-4 left-1/2 -translate-x-1/2 bg-brand-accent text-black px-4 py-1 rounded-full text-sm font-bold shadow-lg"
            >
                BEST VALUE
            </div>

            <div class="mb-6">
                <h3 class="text-2xl font-bold text-brand-text">Pro</h3>
                <p class="text-brand-text/60 mt-2">
                    Unlimited storage and a physical device to record lectures.
                </p>
            </div>

            <div class="mb-8">
                <span class="text-5xl font-bold text-brand-text">$49</span>
                <span class="text-brand-text/50 ml-1">one-time</span>
                <div class="text-sm text-green-500 mt-1 font-medium">
                    Pay once, yours forever
                </div>
            </div>

            <ul class="space-y-4 mb-8 flex-1">
                <li class="flex items-start gap-3 text-brand-text">
                    <div class="bg-brand-accent/20 p-1 rounded-full text-brand-accent mt-0.5">
                        <Icon icon="mdi:check" class="text-xs" />
                    </div>
                    <span>Everything in Free</span>
                </li>
                <li class="flex items-start gap-3 text-brand-text">
                    <div class="bg-brand-accent/20 p-1 rounded-full text-brand-accent mt-0.5">
                        <Icon icon="mdi:check" class="text-xs" />
                    </div>
                    <span><strong>Unlimited</strong> source storage</span>
                </li>
                <li class="flex items-start gap-3 text-brand-text">
                    <div class="bg-brand-accent/20 p-1 rounded-full text-brand-accent mt-0.5">
                        <Icon icon="mdi:check" class="text-xs" />
                    </div>
                    <span><strong>LearningBuddy device</strong> included</span>
                </li>
                <li class="flex items-start gap-3 text-brand-text">
                    <div class="bg-brand-accent/20 p-1 rounded-full text-brand-accent mt-0.5">
                        <Icon icon="mdi:check" class="text-xs" />
                    </div>
                    <span>Live lecture recording & auto-transcription</span>
                </li>
                <li class="flex items-start gap-3 text-brand-text">
                    <div class="bg-brand-accent/20 p-1 rounded-full text-brand-accent mt-0.5">
                        <Icon icon="mdi:check" class="text-xs" />
                    </div>
                    <span>Priority support</span>
                </li>
            </ul>

            <a
                href="/register"
                class="w-full py-4 rounded-xl bg-brand-accent text-black font-bold text-lg hover:scale-[1.02] active:scale-[0.98] transition-all shadow-lg shadow-brand-accent/20 text-center block"
            >
                Get Pro
            </a>
        </div>
    </div>

    <!-- FAQ / Bottom Note -->
    <div class="mt-20 max-w-3xl mx-auto">
        <h2 class="text-2xl font-bold text-brand-text mb-8 text-center">Frequently Asked Questions</h2>
        <div class="space-y-6">
            <div class="bg-brand-surface/30 backdrop-blur-md rounded-2xl border border-brand-text/5 p-6">
                <h3 class="text-lg font-bold text-brand-text mb-2">Why a one-time purchase?</h3>
                <p class="text-brand-text/60 leading-relaxed">
                    We believe students shouldn't worry about recurring charges. Pay once for the Pro plan and you own it forever -- the device, the storage, everything.
                </p>
            </div>
            <div class="bg-brand-surface/30 backdrop-blur-md rounded-2xl border border-brand-text/5 p-6">
                <h3 class="text-lg font-bold text-brand-text mb-2">What's included with the device?</h3>
                <p class="text-brand-text/60 leading-relaxed">
                    The Pro plan ships with a LearningBuddy ESP32-S3 device, pre-configured and ready to pair. Just power it on, connect to Wi-Fi, and enter the 6-character key on your dashboard.
                </p>
            </div>
            <div class="bg-brand-surface/30 backdrop-blur-md rounded-2xl border border-brand-text/5 p-6">
                <h3 class="text-lg font-bold text-brand-text mb-2">Can I upgrade later?</h3>
                <p class="text-brand-text/60 leading-relaxed">
                    Absolutely. Start on the Free plan and upgrade to Pro whenever you're ready. Your existing sources and chat history carry over.
                </p>
            </div>
        </div>
    </div>
</div>

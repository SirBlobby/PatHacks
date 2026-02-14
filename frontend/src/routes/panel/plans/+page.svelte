<script lang="ts">
    import { onMount } from "svelte";

    let billingPeriod = $state("monthly"); // 'monthly' | 'yearly'
</script>

<div class="max-w-6xl mx-auto w-full px-4 py-8">
    <div class="text-center mb-12">
        <h1 class="text-4xl font-bold text-brand-text mb-4">
            Simple, Transparent Pricing
        </h1>
        <p class="text-xl text-brand-text/60 max-w-2xl mx-auto">
            Choose the plan that's right for you. Upgrade or downgrade at any
            time.
        </p>

        <!-- Billing Toggle -->
        <div
            class="mt-8 flex items-center justify-center gap-4 bg-brand-surface/50 w-fit mx-auto p-1 rounded-full border border-brand-text/10"
        >
            <button
                onclick={() => (billingPeriod = "monthly")}
                class="px-6 py-2 rounded-full text-sm font-bold transition-all {billingPeriod ===
                'monthly'
                    ? 'bg-brand-accent text-black shadow-lg'
                    : 'text-brand-text/70 hover:text-brand-text'}"
            >
                Monthly
            </button>
            <button
                onclick={() => (billingPeriod = "yearly")}
                class="px-6 py-2 rounded-full text-sm font-bold transition-all flex items-center gap-2 {billingPeriod ===
                'yearly'
                    ? 'bg-brand-accent text-black shadow-lg'
                    : 'text-brand-text/70 hover:text-brand-text'}"
            >
                Yearly <span
                    class="text-[10px] bg-green-500 text-white px-1.5 py-0.5 rounded-full"
                    >SAVE 20%</span
                >
            </button>
        </div>
    </div>

    <div class="grid md:grid-cols-2 gap-8 max-w-4xl mx-auto">
        <!-- Free Plan -->
        <div
            class="bg-brand-surface/80 backdrop-blur border-2 border-brand-text/10 rounded-3xl p-8 relative hover:border-brand-text/20 transition-colors flex flex-col"
        >
            <div class="mb-6">
                <h3 class="text-2xl font-bold text-brand-text">Free</h3>
                <p class="text-brand-text/60 mt-2">
                    Perfect for getting started with lecture recordings.
                </p>
            </div>

            <div class="mb-8">
                <span class="text-4xl font-bold text-brand-text">$0</span>
                <span class="text-brand-text/50">/ month</span>
            </div>

            <ul class="space-y-4 mb-8 flex-1">
                <li class="flex items-start gap-3 text-brand-text/80">
                    <svg
                        class="w-5 h-5 text-green-500 mt-0.5 shrink-0"
                        fill="none"
                        viewBox="0 0 24 24"
                        stroke="currentColor"
                        ><path
                            stroke-linecap="round"
                            stroke-linejoin="round"
                            stroke-width="2"
                            d="M5 13l4 4L19 7"
                        /></svg
                    >
                    <span>Up to 3 hours of recording/month</span>
                </li>
                <li class="flex items-start gap-3 text-brand-text/80">
                    <svg
                        class="w-5 h-5 text-green-500 mt-0.5 shrink-0"
                        fill="none"
                        viewBox="0 0 24 24"
                        stroke="currentColor"
                        ><path
                            stroke-linecap="round"
                            stroke-linejoin="round"
                            stroke-width="2"
                            d="M5 13l4 4L19 7"
                        /></svg
                    >
                    <span>Basic AI Summaries</span>
                </li>
                <li class="flex items-start gap-3 text-brand-text/80">
                    <svg
                        class="w-5 h-5 text-green-500 mt-0.5 shrink-0"
                        fill="none"
                        viewBox="0 0 24 24"
                        stroke="currentColor"
                        ><path
                            stroke-linecap="round"
                            stroke-linejoin="round"
                            stroke-width="2"
                            d="M5 13l4 4L19 7"
                        /></svg
                    >
                    <span>Web Access Only</span>
                </li>
                <li class="flex items-start gap-3 text-brand-text/80">
                    <svg
                        class="w-5 h-5 text-green-500 mt-0.5 shrink-0"
                        fill="none"
                        viewBox="0 0 24 24"
                        stroke="currentColor"
                        ><path
                            stroke-linecap="round"
                            stroke-linejoin="round"
                            stroke-width="2"
                            d="M5 13l4 4L19 7"
                        /></svg
                    >
                    <span>1 Device Limit</span>
                </li>
            </ul>

            <button
                class="w-full py-3 rounded-xl border-2 border-brand-text/10 text-brand-text font-bold hover:bg-brand-text/5 transition-colors cursor-pointer"
            >
                Current Plan
            </button>
        </div>

        <!-- Pro Plan -->
        <div
            class="bg-brand-surface border-2 border-brand-accent rounded-3xl p-8 relative shadow-2xl shadow-brand-accent/10 flex flex-col transform md:-translate-y-4"
        >
            <div
                class="absolute -top-4 left-1/2 -translate-x-1/2 bg-brand-accent text-black px-4 py-1 rounded-full text-sm font-bold shadow-lg"
            >
                MOST POPULAR
            </div>

            <div class="mb-6">
                <h3 class="text-2xl font-bold text-brand-text">Pro</h3>
                <p class="text-brand-text/60 mt-2">
                    For serious students who need unlimited power.
                </p>
            </div>

            <div class="mb-8">
                <span class="text-5xl font-bold text-brand-text"
                    >${billingPeriod === "monthly" ? "12" : "9"}</span
                >
                <span class="text-brand-text/50">/ month</span>
                {#if billingPeriod === "yearly"}
                    <div class="text-sm text-green-500 mt-1 font-medium">
                        Billed $108 yearly
                    </div>
                {/if}
            </div>

            <ul class="space-y-4 mb-8 flex-1">
                <li class="flex items-start gap-3 text-brand-text">
                    <div
                        class="bg-brand-accent/20 p-1 rounded-full text-brand-accent mt-0.5"
                    >
                        <svg
                            class="w-3 h-3"
                            fill="none"
                            viewBox="0 0 24 24"
                            stroke="currentColor"
                            ><path
                                stroke-linecap="round"
                                stroke-linejoin="round"
                                stroke-width="3"
                                d="M5 13l4 4L19 7"
                            /></svg
                        >
                    </div>
                    <span><strong>Unlimited</strong> recording hours</span>
                </li>
                <li class="flex items-start gap-3 text-brand-text">
                    <div
                        class="bg-brand-accent/20 p-1 rounded-full text-brand-accent mt-0.5"
                    >
                        <svg
                            class="w-3 h-3"
                            fill="none"
                            viewBox="0 0 24 24"
                            stroke="currentColor"
                            ><path
                                stroke-linecap="round"
                                stroke-linejoin="round"
                                stroke-width="3"
                                d="M5 13l4 4L19 7"
                            /></svg
                        >
                    </div>
                    <span>Advanced AI (GPT-4 Class) Summaries</span>
                </li>
                <li class="flex items-start gap-3 text-brand-text">
                    <div
                        class="bg-brand-accent/20 p-1 rounded-full text-brand-accent mt-0.5"
                    >
                        <svg
                            class="w-3 h-3"
                            fill="none"
                            viewBox="0 0 24 24"
                            stroke="currentColor"
                            ><path
                                stroke-linecap="round"
                                stroke-linejoin="round"
                                stroke-width="3"
                                d="M5 13l4 4L19 7"
                            /></svg
                        >
                    </div>
                    <span>Unlimited Chat with Lecture History</span>
                </li>
                <li class="flex items-start gap-3 text-brand-text">
                    <div
                        class="bg-brand-accent/20 p-1 rounded-full text-brand-accent mt-0.5"
                    >
                        <svg
                            class="w-3 h-3"
                            fill="none"
                            viewBox="0 0 24 24"
                            stroke="currentColor"
                            ><path
                                stroke-linecap="round"
                                stroke-linejoin="round"
                                stroke-width="3"
                                d="M5 13l4 4L19 7"
                            /></svg
                        >
                    </div>
                    <span>Priority Support</span>
                </li>
                <li class="flex items-start gap-3 text-brand-text">
                    <div
                        class="bg-brand-accent/20 p-1 rounded-full text-brand-accent mt-0.5"
                    >
                        <svg
                            class="w-3 h-3"
                            fill="none"
                            viewBox="0 0 24 24"
                            stroke="currentColor"
                            ><path
                                stroke-linecap="round"
                                stroke-linejoin="round"
                                stroke-width="3"
                                d="M5 13l4 4L19 7"
                            /></svg
                        >
                    </div>
                    <span>Connect up to 5 Devices</span>
                </li>
            </ul>

            <button
                class="w-full py-4 rounded-xl bg-brand-accent text-black font-bold text-lg hover:scale-[1.02] active:scale-[0.98] transition-all shadow-lg shadow-brand-accent/20 cursor-pointer"
            >
                Upgrade to Pro
            </button>
        </div>
    </div>

    <div class="mt-16 text-center">
        <p class="text-brand-text/50 text-sm">
            Questions? Contact our <a
                href="#"
                class="text-brand-accent hover:underline">support team</a
            >.
        </p>
    </div>
</div>

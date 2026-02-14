<script lang="ts">
    import { page } from "$app/stores";
    let activeTab = $state("summary");
</script>

<div class="max-w-4xl mx-auto w-full">
    <div
        class="flex flex-col md:flex-row justify-between md:items-center mb-8 gap-4"
    >
        <div class="flex items-center gap-4">
            <a
                href="/panel/recordings"
                class="text-brand-text/50 hover:text-brand-text transition flex items-center gap-1 group"
            >
                <span class="group-hover:-translate-x-1 transition-transform"
                    >←</span
                > Back</a
            >
            <h1
                class="text-2xl md:text-3xl font-bold text-brand-text line-clamp-1"
            >
                Lecture #{$page.params.id}: Advanced Topics
            </h1>
        </div>
        <div class="flex gap-2">
            <button
                class="px-4 py-2 rounded-lg bg-brand-surface border border-brand-text/10 text-brand-text hover:bg-brand-text/5 transition shadow-sm"
                >Download PDF</button
            >
            <a
                href="/panel/recordings/{$page.params.id}/chat"
                class="px-4 py-2 rounded-lg bg-brand-accent text-black font-bold hover:opacity-90 transition flex items-center gap-2 shadow-lg shadow-brand-accent/20"
                >Ask Questions</a
            >
        </div>
    </div>

    <div class="flex mb-6 border-b border-brand-text/10 overflow-x-auto">
        <button
            class="px-6 py-3 font-medium transition-colors border-b-2 whitespace-nowrap {activeTab ===
            'summary'
                ? 'border-brand-accent text-brand-accent'
                : 'border-transparent text-brand-text/50 hover:text-brand-text'}"
            onclick={() => (activeTab = "summary")}>Summary</button
        >
        <button
            class="px-6 py-3 font-medium transition-colors border-b-2 whitespace-nowrap {activeTab ===
            'transcript'
                ? 'border-brand-accent text-brand-accent'
                : 'border-transparent text-brand-text/50 hover:text-brand-text'}"
            onclick={() => (activeTab = "transcript")}>Full Transcript</button
        >
    </div>

    {#if activeTab === "summary"}
        <div
            class="p-8 bg-brand-surface backdrop-blur border-2 border-brand-text/10 rounded-xl prose prose-neutral dark:prose-invert max-w-none shadow-lg text-brand-text"
        >
            <h3 class="text-brand-text font-bold">Key Takeaways</h3>
            <ul class="text-brand-text/80">
                <li>Overview of quantum mechanics principles.</li>
                <li>Detailed discussion on Schrödinger's equation.</li>
                <li>Applications in modern computing.</li>
            </ul>
            <h3 class="text-brand-text font-bold">Detailed Summary</h3>
            <p class="text-brand-text/80">
                The lecture began with an introduction to...
            </p>
        </div>
    {:else}
        <div
            class="p-8 bg-brand-surface backdrop-blur border-2 border-brand-text/10 rounded-xl h-[60vh] overflow-y-auto font-mono text-sm leading-relaxed text-brand-text/80 scrollbar-thin scrollbar-thumb-brand-text/20 scrollbar-track-transparent shadow-lg"
        >
            <p class="mb-4">
                <span class="text-brand-text/40 select-none mr-2"
                    >[00:00:12]</span
                >
                <strong class="text-brand-accent">Professor:</strong> Welcome everyone
                to today's lecture on...
            </p>
            <p class="mb-4">
                <span class="text-brand-text/40 select-none mr-2"
                    >[00:01:45]</span
                >
                <strong class="text-blue-500 dark:text-blue-400"
                    >Student:</strong
                > Could you clarify the previous point regarding the wave function
                collapse?
            </p>
            <p class="mb-4">
                <span class="text-brand-text/40 select-none mr-2"
                    >[00:02:00]</span
                >
                <strong class="text-brand-accent">Professor:</strong> Absolutely.
                So essentially, the Copenhagen interpretation suggests...
            </p>
            <!-- More transcript placeholder -->
        </div>
    {/if}
</div>

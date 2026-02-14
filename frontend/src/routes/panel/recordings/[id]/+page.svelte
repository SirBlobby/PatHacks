<script lang="ts">
    import { page } from "$app/stores";
    let activeTab = $state("summary");
</script>

<div class="flex justify-between items-center mb-8">
    <div class="flex items-center gap-4">
        <a
            href="/panel/recordings"
            class="text-gray-400 hover:text-white transition">← Back</a
        >
        <h1 class="text-3xl font-bold text-white">
            Lecture #{$page.params.id}: Advanced Topics
        </h1>
    </div>
    <div class="flex gap-2">
        <button
            class="px-4 py-2 rounded-lg bg-gray-800 text-white hover:bg-gray-700 transition"
            >Download PDF</button
        >
        <a
            href="/panel/recordings/{$page.params.id}/chat"
            class="px-4 py-2 rounded-lg bg-brand-accent text-black font-bold hover:opacity-90 transition flex items-center gap-2"
            >Ask Questions</a
        >
    </div>
</div>

<div class="flex mb-6 border-b border-gray-800">
    <button
        class="px-6 py-3 font-medium transition-colors border-b-2 {activeTab ===
        'summary'
            ? 'border-brand-accent text-brand-accent'
            : 'border-transparent text-gray-400 hover:text-white'}"
        onclick={() => (activeTab = "summary")}>Summary</button
    >
    <button
        class="px-6 py-3 font-medium transition-colors border-b-2 {activeTab ===
        'transcript'
            ? 'border-brand-accent text-brand-accent'
            : 'border-transparent text-gray-400 hover:text-white'}"
        onclick={() => (activeTab = "transcript")}>Full Transcript</button
    >
</div>

{#if activeTab === "summary"}
    <div
        class="p-8 bg-gray-900/50 border border-gray-800 rounded-xl prose prose-invert max-w-none"
    >
        <h3>Key Takeaways</h3>
        <ul>
            <li>Overview of quantum mechanics principles.</li>
            <li>Detailed discussion on Schrödinger's equation.</li>
            <li>Applications in modern computing.</li>
        </ul>
        <h3>Detailed Summary</h3>
        <p>The lecture began with an introduction to...</p>
    </div>
{:else}
    <div
        class="p-8 bg-gray-900/50 border border-gray-800 rounded-xl h-[60vh] overflow-y-auto font-mono text-sm leading-relaxed text-gray-300 scrollbar-thin scrollbar-thumb-gray-700 scrollbar-track-transparent"
    >
        <p class="mb-4">
            <span class="text-gray-500 select-none mr-2">[00:00:12]</span>
            <strong class="text-brand-accent">Professor:</strong> Welcome everyone
            to today's lecture on...
        </p>
        <p class="mb-4">
            <span class="text-gray-500 select-none mr-2">[00:01:45]</span>
            <strong class="text-blue-400">Student:</strong> Could you clarify the
            previous point regarding the wave function collapse?
        </p>
        <p class="mb-4">
            <span class="text-gray-500 select-none mr-2">[00:02:00]</span>
            <strong class="text-brand-accent">Professor:</strong> Absolutely. So
            essentially, the Copenhagen interpretation suggests...
        </p>
        <!-- More transcript placeholder -->
    </div>
{/if}

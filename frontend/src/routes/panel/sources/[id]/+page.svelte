<script lang="ts">
    import { page } from "$app/stores";
    import { onMount } from "svelte";
    import { getSource, regenerateSummary } from "$lib/api";
    import Icon from "@iconify/svelte";

    let loading = $state(true);
    let error = $state("");
    let source = $state(null as any);
    let activeTab = $state("summary");
    let regenerating = $state(false);

    const typeIcons: Record<string, string> = {
        text: "mdi:file-document-outline",
        pdf: "mdi:file-pdf-box",
        docx: "mdi:file-word-box",
        audio: "mdi:headphones",
        video: "mdi:filmstrip",
        image: "mdi:image-outline",
        code: "mdi:code-tags",
        csv: "mdi:file-delimited",
        excel: "mdi:file-excel-box",
        markdown: "mdi:language-markdown",
        json: "mdi:code-json",
        yaml: "mdi:cog-outline",
        xml: "mdi:xml",
        html: "mdi:language-html5",
    };

    onMount(async () => {
        try {
            source = await getSource($page.params.id as string);
        } catch (err: any) {
            error = err.message;
        } finally {
            loading = false;
        }
    });

    async function handleRegenerate() {
        regenerating = true;
        try {
            const data = await regenerateSummary($page.params.id as string);
            source = { ...source, summary: data.summary };
        } catch (err: any) {
            alert(err.message);
        } finally {
            regenerating = false;
        }
    }

    function formatTime(seconds: number): string {
        const m = Math.floor(seconds / 60);
        const s = Math.floor(seconds % 60);
        return `${m.toString().padStart(2, "0")}:${s.toString().padStart(2, "0")}`;
    }
</script>

{#if loading}
    <div class="flex justify-center py-20 max-w-4xl mx-auto">
        <div
            class="w-10 h-10 border-4 border-brand-accent border-t-transparent rounded-full animate-spin"
        ></div>
    </div>
{:else if error}
    <div class="max-w-4xl mx-auto">
        <div
            class="p-4 bg-red-500/10 border border-red-500/30 rounded-xl text-red-500 text-center flex flex-col items-center gap-2"
        >
            <Icon icon="mdi:alert-circle-outline" class="text-3xl" />
            {error}
        </div>
    </div>
{:else if source}
    <div class="max-w-4xl mx-auto w-full">
        <div
            class="flex flex-col md:flex-row justify-between md:items-center mb-8 gap-4"
        >
            <div class="flex items-center gap-4">
                <a
                    href="/panel/sources"
                    class="text-brand-text/50 hover:text-brand-text transition flex items-center gap-1 group"
                >
                    <Icon
                        icon="mdi:arrow-left"
                        class="group-hover:-translate-x-1 transition-transform"
                    />
                    Back</a
                >
                <div
                    class="w-12 h-12 rounded-xl bg-brand-text/5 flex items-center justify-center shrink-0 text-brand-accent"
                >
                    <Icon
                        icon={typeIcons[source.file_type] ||
                            "mdi:file-document-outline"}
                        class="text-3xl"
                    />
                </div>
                <div>
                    <h1
                        class="text-2xl md:text-3xl font-bold text-brand-text line-clamp-1"
                    >
                        {source.title}
                    </h1>
                    <p
                        class="text-sm text-brand-text/40 mt-0.5 flex items-center gap-2"
                    >
                        <span
                            class="uppercase tracking-wider font-semibold text-brand-accent"
                            >{source.file_type}</span
                        >
                        <span>•</span>
                        <span
                            >{source.char_count?.toLocaleString() || 0} chars</span
                        >
                        {#if source.original_filename}
                            <span>•</span>
                            <span class="truncate max-w-[200px]"
                                >{source.original_filename}</span
                            >
                        {/if}
                    </p>
                </div>
            </div>
            <div class="flex items-center gap-3">
                <button
                    onclick={handleRegenerate}
                    disabled={regenerating}
                    class="h-10 px-4 rounded-lg bg-brand-surface border border-brand-text/10 text-brand-text hover:bg-brand-text/5 transition shadow-sm disabled:opacity-50 flex items-center justify-center gap-2 text-sm font-medium whitespace-nowrap"
                >
                    <Icon
                        icon="mdi:refresh"
                        class="text-lg {regenerating ? 'animate-spin' : ''}"
                    />
                    <span
                        >{regenerating
                            ? "Regenerating..."
                            : "Regenerate Summary"}</span
                    >
                </button>
                <a
                    href="/panel/sources/{$page.params.id}/chat"
                    class="h-10 px-5 rounded-lg bg-brand-accent text-black font-bold hover:opacity-90 transition flex items-center justify-center gap-2 shadow-lg shadow-brand-accent/20 text-sm whitespace-nowrap"
                >
                    <Icon icon="mdi:chat-processing-outline" class="text-xl" />
                    <span>Ask Questions</span>
                </a>
            </div>
        </div>

        <div class="flex mb-6 border-b border-brand-text/10 overflow-x-auto">
            <button
                class="px-6 py-3 font-medium transition-colors border-b-2 whitespace-nowrap flex items-center gap-2 {activeTab ===
                'summary'
                    ? 'border-brand-accent text-brand-accent'
                    : 'border-transparent text-brand-text/50 hover:text-brand-text'}"
                onclick={() => (activeTab = "summary")}
            >
                <Icon icon="mdi:text-box-check-outline" class="text-lg" />
                Summary
            </button>
            <button
                class="px-6 py-3 font-medium transition-colors border-b-2 whitespace-nowrap flex items-center gap-2 {activeTab ===
                'content'
                    ? 'border-brand-accent text-brand-accent'
                    : 'border-transparent text-brand-text/50 hover:text-brand-text'}"
                onclick={() => (activeTab = "content")}
            >
                <Icon
                    icon="mdi:file-document-content-outline"
                    class="text-lg"
                />
                Full Content
            </button>
        </div>

        {#if activeTab === "summary"}
            <div
                class="p-8 bg-brand-surface backdrop-blur border-2 border-brand-text/10 rounded-xl prose prose-neutral dark:prose-invert max-w-none shadow-lg text-brand-text"
            >
                {#if source.summary}
                    {@html source.summary
                        .replace(
                            /^## (.+)$/gm,
                            '<h3 class="text-brand-text font-bold">$1</h3>',
                        )
                        .replace(
                            /^- (.+)$/gm,
                            '<li class="text-brand-text/80">$1</li>',
                        )
                        .replace(/\n/g, "<br>")}
                {:else}
                    <div
                        class="flex flex-col items-center justify-center py-12 text-brand-text/50"
                    >
                        {#if source.status === "processing"}
                            <div
                                class="w-12 h-12 border-4 border-brand-accent border-t-transparent rounded-full animate-spin mb-4"
                            ></div>
                            <p>Summary is being generated...</p>
                        {:else if source.status === "transcribing"}
                            <div
                                class="w-12 h-12 border-4 border-brand-accent border-t-transparent rounded-full animate-spin mb-4"
                            ></div>
                            <p>Audio/video is being transcribed...</p>
                        {:else}
                            <Icon
                                icon="mdi:text-box-remove-outline"
                                class="text-5xl mb-2 op-50"
                            />
                            <p>
                                No summary available. Click 'Regenerate Summary'
                                to create one.
                            </p>
                        {/if}
                    </div>
                {/if}
            </div>
        {:else}
            <div
                class="p-4 md:p-8 bg-brand-surface backdrop-blur border-2 border-brand-text/10 rounded-xl max-h-[70vh] min-h-[50vh] overflow-y-auto scrollbar-thin scrollbar-thumb-brand-text/20 scrollbar-track-transparent shadow-lg text-brand-text"
            >
                {#if source.transcript_segments && source.transcript_segments.length > 0}
                    <div class="space-y-1">
                        {#each source.transcript_segments as segment}
                            <div
                                class="flex gap-4 hover:bg-white/5 p-2 rounded-lg transition-colors group"
                            >
                                <span
                                    class="text-brand-accent/60 font-mono text-xs shrink-0 pt-1.5 min-w-[50px] select-none group-hover:text-brand-accent transition-colors"
                                >
                                    {formatTime(segment.start)}
                                </span>
                                <p
                                    class="text-brand-text/90 leading-relaxed text-base"
                                >
                                    {segment.text}
                                </p>
                            </div>
                        {/each}
                    </div>
                {:else if source.content}
                    <div
                        class="whitespace-pre-wrap font-sans text-brand-text/80 leading-7 max-w-none prose prose-invert"
                    >
                        {source.content}
                    </div>
                {:else}
                    <div
                        class="flex flex-col items-center justify-center h-full text-brand-text/50 py-20"
                    >
                        <Icon icon="mdi:file-hidden" class="text-5xl mb-2" />
                        <p>No content available.</p>
                    </div>
                {/if}
            </div>
        {/if}
    </div>
{/if}

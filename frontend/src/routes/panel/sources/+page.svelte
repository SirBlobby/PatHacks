<script lang="ts">
    import { onMount } from "svelte";
    import { listSources, deleteSource } from "$lib/api";
    import Icon from "@iconify/svelte";

    let loading = $state(true);
    let error = $state("");
    let sources = $state([] as any[]);
    let searchQuery = $state("");
    let searchTimeout: ReturnType<typeof setTimeout>;

    // File type icons (Iconify names)
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
        await fetchSources();
    });

    async function fetchSources(query?: string) {
        loading = true;
        error = "";
        try {
            sources = await listSources(query);
        } catch (err: any) {
            error = err.message;
        } finally {
            loading = false;
        }
    }

    function handleSearch() {
        clearTimeout(searchTimeout);
        searchTimeout = setTimeout(() => {
            fetchSources(searchQuery);
        }, 300);
    }

    async function handleDelete(id: string, title: string) {
        if (!confirm(`Delete source "${title}"? This cannot be undone.`))
            return;
        try {
            await deleteSource(id);
            sources = sources.filter((s: any) => s.id !== id);
        } catch (err: any) {
            alert(err.message);
        }
    }

    function formatDate(dateStr: string | null): string {
        if (!dateStr) return "—";
        return new Date(dateStr).toLocaleDateString("en-US", {
            month: "short",
            day: "numeric",
            year: "numeric",
        });
    }

    function formatSize(charCount: number): string {
        if (!charCount) return "";
        if (charCount < 1000) return `${charCount} chars`;
        if (charCount < 1_000_000)
            return `${(charCount / 1000).toFixed(1)}K chars`;
        return `${(charCount / 1_000_000).toFixed(1)}M chars`;
    }
</script>

<div class="max-w-4xl mx-auto w-full">
    <div class="flex justify-between items-center mb-8">
        <h1 class="text-3xl font-bold text-brand-text">Sources</h1>
        <a
            href="/panel/sources/new"
            class="px-5 py-2.5 bg-brand-accent text-black rounded-xl font-semibold hover:brightness-110 transition shadow-lg shadow-brand-accent/20 flex items-center gap-2"
        >
            <Icon icon="mdi:plus" class="text-lg" />
            Add Source
        </a>
    </div>

    <!-- Search -->
    <div class="mb-6 relative">
        <div
            class="absolute inset-y-0 left-0 pl-3 flex items-center pointer-events-none"
        >
            <Icon icon="mdi:magnify" class="text-brand-text/30 text-xl" />
        </div>
        <input
            type="text"
            bind:value={searchQuery}
            oninput={handleSearch}
            placeholder="Search sources..."
            class="w-full bg-brand-surface border border-brand-text/10 rounded-xl pl-10 pr-4 py-3 text-brand-text placeholder-brand-text/30 focus:border-brand-accent focus:ring-1 focus:ring-brand-accent outline-none transition-all"
        />
    </div>

    {#if loading}
        <div class="flex justify-center py-20">
            <div
                class="w-10 h-10 border-4 border-brand-accent border-t-transparent rounded-full animate-spin"
            ></div>
        </div>
    {:else if error}
        <div
            class="p-4 bg-red-500/10 border border-red-500/30 rounded-xl text-red-500 text-center flex flex-col items-center gap-2"
        >
            <Icon icon="mdi:alert-circle-outline" class="text-3xl" />
            {error}
        </div>
    {:else if sources.length === 0}
        <div
            class="bg-brand-surface/50 border border-brand-text/10 rounded-2xl p-12 text-center flex flex-col items-center"
        >
            <Icon
                icon="mdi:bookshelf"
                class="text-6xl text-brand-text/20 mb-4"
            />
            <p class="text-brand-text/50 text-lg">
                {searchQuery
                    ? "No sources match your search"
                    : "No sources yet. Upload a file or paste text to get started."}
            </p>
            {#if !searchQuery}
                <a
                    href="/panel/sources/new"
                    class="inline-flex items-center gap-2 mt-6 px-6 py-3 bg-brand-accent text-black rounded-xl font-semibold hover:brightness-110 transition"
                >
                    <Icon icon="mdi:plus-circle-outline" class="text-xl" />
                    Add Your First Source
                </a>
            {/if}
        </div>
    {:else}
        <div class="space-y-3">
            {#each sources as src}
                <div
                    class="bg-brand-surface/60 backdrop-blur border border-brand-text/10 rounded-xl p-4 hover:border-brand-text/20 transition-colors group flex items-center justify-between"
                >
                    <a href="/panel/sources/{src.id}" class="flex-1 min-w-0">
                        <div class="flex items-center justify-between gap-4">
                            <div class="flex items-center gap-3 min-w-0">
                                <div
                                    class="w-10 h-10 rounded-lg bg-brand-text/5 flex items-center justify-center shrink-0 text-brand-accent"
                                >
                                    <Icon
                                        icon={typeIcons[src.file_type] ||
                                            "mdi:file-document-outline"}
                                        class="text-2xl"
                                    />
                                </div>
                                <div class="min-w-0">
                                    <p
                                        class="font-medium text-brand-text group-hover:text-brand-accent transition-colors truncate"
                                    >
                                        {src.title}
                                    </p>
                                    <p
                                        class="text-sm text-brand-text/50 mt-0.5 flex items-center gap-2"
                                    >
                                        <span>{formatDate(src.created_at)}</span
                                        >
                                        {#if src.char_count}
                                            <span
                                                class="w-1 h-1 rounded-full bg-brand-text/30"
                                            ></span>
                                            <span
                                                >{formatSize(
                                                    src.char_count,
                                                )}</span
                                            >
                                        {/if}
                                        <span
                                            class="w-1 h-1 rounded-full bg-brand-text/30"
                                        ></span>
                                        <span
                                            class="uppercase text-xs tracking-wider"
                                            >{src.file_type}</span
                                        >
                                    </p>
                                </div>
                            </div>
                            <span
                                class="text-xs px-2 py-1 rounded-full font-medium shrink-0 flex items-center gap-1 {src.status ===
                                'processed'
                                    ? 'bg-green-500/10 text-green-500'
                                    : src.status === 'error'
                                      ? 'bg-red-500/10 text-red-500'
                                      : 'bg-yellow-500/10 text-yellow-500'}"
                            >
                                {#if src.status === "processed"}
                                    <Icon icon="mdi:check-circle-outline" />
                                {:else if src.status === "error"}
                                    <Icon icon="mdi:alert-circle-outline" />
                                {:else}
                                    <Icon icon="mdi:clock-outline" />
                                {/if}
                                {src.status}
                            </span>
                        </div>
                    </a>
                    <button
                        onclick={() => handleDelete(src.id, src.title)}
                        class="ml-3 p-2 rounded-lg hover:bg-red-500/10 text-brand-text/20 hover:text-red-500 transition-colors shrink-0"
                        title="Delete"
                    >
                        <Icon icon="mdi:trash-can-outline" class="text-xl" />
                    </button>
                </div>
            {/each}
        </div>
    {/if}
</div>

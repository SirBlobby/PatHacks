<script lang="ts">
    import { goto } from "$app/navigation";
    import { createSourceText, uploadSourceFile } from "$lib/api";
    import Icon from "@iconify/svelte";
    import { deskpet } from "$lib/stores/deskpet";

    let mode = $state<"upload" | "text">("upload");
    let title = $state("");
    let content = $state("");
    let file = $state<File | null>(null);
    let loading = $state(false);
    let error = $state("");
    let dragOver = $state(false);

    function handleFileSelect(e: Event) {
        const input = e.target as HTMLInputElement;
        if (input.files?.[0]) {
            file = input.files[0];
            if (!title) {
                title = file.name.replace(/\.[^.]+$/, "");
            }
        }
    }

    function handleDrop(e: DragEvent) {
        e.preventDefault();
        dragOver = false;
        if (e.dataTransfer?.files?.[0]) {
            file = e.dataTransfer.files[0];
            if (!title) {
                title = file.name.replace(/\.[^.]+$/, "");
            }
        }
    }

    function handleDragOver(e: DragEvent) {
        e.preventDefault();
        dragOver = true;
    }

    function handleDragLeave() {
        dragOver = false;
    }

    function formatFileSize(bytes: number): string {
        if (bytes < 1024) return `${bytes} B`;
        if (bytes < 1048576) return `${(bytes / 1024).toFixed(1)} KB`;
        return `${(bytes / 1048576).toFixed(1)} MB`;
    }

    async function handleSubmit() {
        error = "";

        if (mode === "upload") {
            if (!file) {
                error = "Please select a file to upload.";
                return;
            }
        } else {
            if (!title.trim()) {
                error = "Title is required.";
                return;
            }
            if (!content.trim()) {
                error = "Content is required.";
                return;
            }
        }

        loading = true;
        try {
            if (mode === "upload") {
                await uploadSourceFile(file!, title || undefined);
            } else {
                await createSourceText({
                    title: title.trim(),
                    content: content.trim(),
                });
            }
            deskpet.setExpression("love", 5000);
            goto("/panel/sources");
        } catch (err: any) {
            error = err.message;
        } finally {
            loading = false;
        }
    }
</script>

<div class="max-w-2xl mx-auto w-full">
    <h1 class="text-3xl font-bold text-brand-text mb-8">Add Source</h1>

    <!-- Mode Switch -->
    <div
        class="flex gap-2 mb-8 bg-brand-surface/60 p-1 rounded-xl border border-brand-text/10 w-fit"
    >
        <button
            onclick={() => (mode = "upload")}
            class="px-5 py-2.5 rounded-lg font-medium transition-all flex items-center gap-2 {mode ===
            'upload'
                ? 'bg-brand-accent text-black shadow-lg'
                : 'text-brand-text/60 hover:text-brand-text'}"
        >
            <Icon icon="mdi:file-upload-outline" class="text-lg" />
            Upload File
        </button>
        <button
            onclick={() => (mode = "text")}
            class="px-5 py-2.5 rounded-lg font-medium transition-all flex items-center gap-2 {mode ===
            'text'
                ? 'bg-brand-accent text-black shadow-lg'
                : 'text-brand-text/60 hover:text-brand-text'}"
        >
            <Icon icon="mdi:text-box-plus-outline" class="text-lg" />
            Paste Text
        </button>
    </div>

    {#if error}
        <div
            class="p-4 mb-6 bg-red-500/10 border border-red-500/30 rounded-xl text-red-500 flex items-center gap-2"
        >
            <Icon icon="mdi:alert-circle-outline" class="text-xl" />
            {error}
        </div>
    {/if}

    <div
        class="bg-brand-surface/60 backdrop-blur border border-brand-text/10 rounded-2xl p-6 space-y-6"
    >
        <!-- Title -->
        <div>
            <label
                for="title"
                class="block text-sm font-medium text-brand-text mb-2"
                >Title</label
            >
            <input
                id="title"
                type="text"
                bind:value={title}
                placeholder={mode === "upload"
                    ? "Auto-filled from filename"
                    : "e.g. Physics Lecture Notes"}
                class="w-full bg-brand-bg border border-brand-text/10 rounded-xl p-3 text-brand-text placeholder-brand-text/30 focus:border-brand-accent focus:ring-1 focus:ring-brand-accent outline-none transition"
            />
        </div>

        {#if mode === "upload"}
            <!-- Drop Zone -->
            <div>
                <label
                    for="file-upload"
                    class="block text-sm font-medium text-brand-text mb-2"
                    >File</label
                >
                <!-- svelte-ignore a11y_no_static_element_interactions -->
                <div
                    ondrop={handleDrop}
                    ondragover={handleDragOver}
                    ondragleave={handleDragLeave}
                    class="border-2 border-dashed rounded-2xl p-10 text-center transition-all cursor-pointer flex flex-col items-center justify-center {dragOver
                        ? 'border-brand-accent bg-brand-accent/5'
                        : 'border-brand-text/20 hover:border-brand-text/40'}"
                >
                    {#if file}
                        <div class="space-y-2 flex flex-col items-center">
                            <Icon
                                icon="mdi:paperclip"
                                class="text-4xl text-brand-accent"
                            />
                            <p class="text-brand-text font-medium">
                                {file.name}
                            </p>
                            <p class="text-sm text-brand-text/50">
                                {formatFileSize(file.size)}
                            </p>
                            <button
                                onclick={() => {
                                    file = null;
                                }}
                                class="text-sm text-red-400 hover:text-red-300 underline mt-2"
                            >
                                Remove
                            </button>
                        </div>
                    {:else}
                        <Icon
                            icon="mdi:cloud-upload-outline"
                            class="text-5xl mb-3 text-brand-text/30"
                        />
                        <p class="text-brand-text/70 mb-2">
                            Drag & drop a file here, or
                            <label
                                class="text-brand-accent cursor-pointer hover:underline"
                            >
                                browse
                                <input
                                    id="file-upload"
                                    type="file"
                                    class="hidden"
                                    onchange={handleFileSelect}
                                    accept=".txt,.md,.pdf,.docx,.doc,.json,.csv,.xlsx,.xls,.yaml,.yml,.xml,.html,.htm,.py,.js,.ts,.java,.cpp,.c,.rs,.go,.rb,.php,.swift,.kt,.sql,.sh,.css,.png,.jpg,.jpeg,.gif,.webp,.mp3,.wav,.m4a,.flac,.ogg,.aac,.mp4,.mkv,.avi,.mov,.webm"
                                />
                            </label>
                        </p>
                        <p class="text-sm text-brand-text/40">
                            PDF, Word, Audio, Video, Code, Images, and more
                        </p>
                    {/if}
                </div>
            </div>
        {:else}
            <!-- Text Content -->
            <div>
                <label
                    for="content"
                    class="block text-sm font-medium text-brand-text mb-2"
                >
                    Content
                </label>
                <textarea
                    id="content"
                    bind:value={content}
                    rows="12"
                    placeholder="Paste your lecture notes, article, transcript, or any text content here..."
                    class="w-full bg-brand-bg border border-brand-text/10 rounded-xl p-4 text-brand-text placeholder-brand-text/30 focus:border-brand-accent focus:ring-1 focus:ring-brand-accent outline-none transition resize-y min-h-[200px]"
                ></textarea>
                {#if content}
                    <p class="text-xs text-brand-text/40 mt-1">
                        {content.length.toLocaleString()} characters
                    </p>
                {/if}
            </div>
        {/if}

        <!-- Submit -->
        <div class="flex gap-4 pt-2">
            <button
                onclick={handleSubmit}
                disabled={loading}
                class="flex-1 py-3 bg-brand-accent text-black rounded-xl font-semibold hover:brightness-110 transition shadow-lg shadow-brand-accent/20 disabled:opacity-50 disabled:cursor-not-allowed"
            >
                {#if loading}
                    <span class="flex items-center justify-center gap-2">
                        <span
                            class="w-4 h-4 border-2 border-black/30 border-t-black rounded-full animate-spin"
                        ></span>
                        Processing...
                    </span>
                {:else}
                    {mode === "upload" ? "Upload & Process" : "Save & Process"}
                {/if}
            </button>
            <a
                href="/panel/sources"
                class="px-6 py-3 border border-brand-text/20 text-brand-text rounded-xl font-medium hover:bg-brand-text/5 transition"
            >
                Cancel
            </a>
        </div>
    </div>
</div>

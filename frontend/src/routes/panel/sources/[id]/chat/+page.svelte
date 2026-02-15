<script lang="ts">
    import { marked } from "marked";
    import { page } from "$app/stores";
    import { onMount } from "svelte";
    import Icon from "@iconify/svelte";
    import {
        getChatHistory,
        streamChatMessage,
        clearChatHistory,
    } from "$lib/api";

    let input = $state("");
    let messages = $state([] as { role: string; content: string }[]);
    let chatContainer = $state<HTMLDivElement | null>(null);
    let loading = $state(true);
    let sending = $state(false);
    let streamingContent = $state("");
    let sourceId = $derived($page.params.id ?? "");

    onMount(async () => {
        try {
            const history = await getChatHistory(sourceId);
            if (history.length > 0) {
                messages = history.map((m: any) => ({
                    role: m.role,
                    content: m.content,
                }));
            } else {
                messages = [
                    {
                        role: "assistant",
                        content:
                            "Hello! I'm ready to answer questions about this source. What would you like to know?",
                    },
                ];
            }
        } catch {
            messages = [
                {
                    role: "assistant",
                    content:
                        "Hello! I'm ready to answer questions about this source. What would you like to know?",
                },
            ];
        } finally {
            loading = false;
            scrollToBottom();
        }
    });

    function scrollToBottom() {
        setTimeout(() => {
            if (chatContainer)
                chatContainer.scrollTop = chatContainer.scrollHeight;
        }, 0);
    }

    async function send() {
        if (!input.trim() || sending) return;

        const userMessage = input.trim();
        messages = [...messages, { role: "user", content: userMessage }];
        input = "";
        sending = true;
        streamingContent = "";
        scrollToBottom();

        // Add a placeholder for the streaming response
        messages = [...messages, { role: "assistant", content: "" }];
        scrollToBottom();

        try {
            const fullResponse = await streamChatMessage(
                sourceId,
                userMessage,
                (chunk: string) => {
                    streamingContent += chunk;
                    // Update the last message with streaming content
                    messages = [
                        ...messages.slice(0, -1),
                        { role: "assistant", content: streamingContent },
                    ];
                    scrollToBottom();
                },
            );

            // Final update with complete response
            messages = [
                ...messages.slice(0, -1),
                { role: "assistant", content: fullResponse },
            ];
        } catch (err: any) {
            messages = [
                ...messages.slice(0, -1),
                {
                    role: "assistant",
                    content: `Sorry, something went wrong: ${err.message}`,
                },
            ];
        } finally {
            sending = false;
            streamingContent = "";
            scrollToBottom();
        }
    }

    async function handleClear() {
        if (!confirm("Clear all chat history for this source?")) return;
        try {
            await clearChatHistory(sourceId);
            messages = [
                {
                    role: "assistant",
                    content:
                        "Chat history cleared. What would you like to know?",
                },
            ];
        } catch (err: any) {
            alert(err.message);
        }
    }
</script>

{#if loading}
    <div class="flex justify-center py-20 max-w-4xl mx-auto">
        <div
            class="w-10 h-10 border-4 border-brand-accent border-t-transparent rounded-full animate-spin"
        ></div>
    </div>
{:else}
    <div class="flex flex-col h-[calc(100vh-6rem)] max-w-4xl mx-auto w-full">
        <div
            class="flex items-center justify-between gap-4 mb-4 pb-4 border-b border-brand-text/10 bg-brand-surface/95 backdrop-blur-xl z-20 sticky top-0 px-2 py-3 rounded-b-xl"
        >
            <div class="flex items-center gap-3">
                <a
                    href="/panel/sources/{sourceId}"
                    class="p-2 rounded-lg hover:bg-brand-text/5 text-brand-text/60 hover:text-brand-text transition group"
                    aria-label="Back to Source"
                >
                    <Icon
                        icon="mdi:arrow-left"
                        class="text-2xl group-hover:-translate-x-1 transition-transform"
                    />
                </a>

                <div
                    class="w-10 h-10 rounded-full bg-brand-accent/10 border border-brand-accent/20 flex items-center justify-center text-brand-accent shrink-0"
                >
                    <Icon icon="mdi:robot-happy-outline" class="text-xl" />
                </div>

                <div>
                    <h1 class="text-lg font-bold text-brand-text leading-tight">
                        Chat with Source
                    </h1>
                    <div class="flex items-center gap-1.5">
                        <span class="relative flex h-2 w-2">
                            <span
                                class="animate-ping absolute inline-flex h-full w-full rounded-full bg-green-400/50 opacity-75"
                            ></span>
                            <span
                                class="relative inline-flex rounded-full h-2 w-2 bg-green-500"
                            ></span>
                        </span>
                        <span class="text-xs font-medium text-brand-text/60"
                            >Powered By Gemini 3 Pro</span
                        >
                    </div>
                </div>
            </div>

            <button
                onclick={handleClear}
                class="p-2 rounded-lg hover:bg-brand-text/5 text-brand-text/40 hover:text-brand-text transition"
                title="Clear chat history"
            >
                <Icon icon="mdi:delete-sweep-outline" class="text-2xl" />
            </button>
        </div>

        <div
            bind:this={chatContainer}
            class="flex-1 overflow-y-auto space-y-6 mb-6 p-6 bg-brand-surface/30 rounded-2xl border border-brand-text/10 scrollbar-thin scrollbar-thumb-brand-text/20 scrollbar-track-transparent shadow-inner"
        >
            {#each messages as msg}
                <div
                    class="flex w-full {msg.role === 'user'
                        ? 'justify-end'
                        : 'justify-start'} animate-fade-in-up"
                >
                    <div class="flex flex-col max-w-[80%]">
                        <span
                            class="text-xs text-brand-text/50 mb-1 ml-1 {msg.role ===
                            'user'
                                ? 'text-right'
                                : 'text-left'}"
                            >{msg.role === "user"
                                ? "You"
                                : "LearningBuddy AI"}</span
                        >
                        <div
                            class="p-4 rounded-2xl shadow-sm border {msg.role ===
                            'user'
                                ? 'bg-brand-accent text-black rounded-tr-none border-transparent'
                                : 'bg-brand-bg text-brand-text rounded-tl-none border-brand-text/10'}"
                        >
                            {#if msg.content}
                                <div
                                    class="prose prose-sm dark:prose-invert max-w-none"
                                >
                                    {@html marked.parse(msg.content)}
                                </div>
                            {:else}
                                <span
                                    class="inline-flex gap-1 items-center text-brand-text/50"
                                >
                                    <span
                                        class="w-2 h-2 bg-brand-accent rounded-full animate-bounce"
                                    ></span>
                                    <span
                                        class="w-2 h-2 bg-brand-accent rounded-full animate-bounce"
                                        style="animation-delay: 0.1s"
                                    ></span>
                                    <span
                                        class="w-2 h-2 bg-brand-accent rounded-full animate-bounce"
                                        style="animation-delay: 0.2s"
                                    ></span>
                                </span>
                            {/if}
                        </div>
                    </div>
                </div>
            {/each}
        </div>

        <form
            onsubmit={(e) => {
                e.preventDefault();
                send();
            }}
            class="flex gap-4 bg-brand-bg/50 backdrop-blur p-2 rounded-2xl border border-brand-text/10 focus-within:border-brand-accent transition-colors shadow-lg relative"
        >
            <input
                type="text"
                bind:value={input}
                placeholder="Ask a question about this source..."
                class="flex-1 bg-transparent px-4 py-3 text-brand-text placeholder-brand-text/50 outline-none"
                disabled={sending}
            />
            <button
                type="submit"
                class="bg-brand-accent text-black font-bold px-6 py-2 rounded-xl hover:scale-105 transition-transform shadow-lg shadow-brand-accent/20 disabled:opacity-50 disabled:cursor-not-allowed cursor-pointer flex items-center gap-2"
                disabled={!input.trim() || sending}
            >
                {#if sending}
                    <Icon icon="mdi:loading" class="animate-spin text-xl" />
                {:else}
                    <span>Send</span>
                    <Icon icon="mdi:send" class="text-lg" />
                {/if}
            </button>
        </form>
    </div>
{/if}

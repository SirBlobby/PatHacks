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
    import VoiceChat from "$lib/components/VoiceChat.svelte";

    let input = $state("");
    let messages = $state([] as { role: string; content: string }[]);
    let chatContainer = $state<HTMLDivElement | null>(null);
    let loading = $state(true);
    let sending = $state(false);
    let streamingContent = $state("");
    let sourceId = $derived($page.params.id ?? "");
    let chatMode = $state<"text" | "voice">("text");

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
                    messages = [
                        ...messages.slice(0, -1),
                        { role: "assistant", content: streamingContent },
                    ];
                    scrollToBottom();
                },
            );

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
        <!-- Header -->
        <div
            class="flex items-center justify-between gap-4 pb-4 border-b border-brand-text/10 mb-4 flex-shrink-0"
        >
            <div class="flex items-center gap-3">
                <a
                    href="/panel/sources/{sourceId}"
                    class="p-2 rounded-lg hover:bg-brand-text/5 text-brand-text/50 hover:text-brand-text transition group"
                    aria-label="Back to Source"
                >
                    <Icon
                        icon="mdi:arrow-left"
                        class="text-xl group-hover:-translate-x-0.5 transition-transform"
                    />
                </a>

                <div>
                    <h1 class="text-lg font-bold text-brand-text leading-tight">
                        {chatMode === "text"
                            ? "Chat with Source"
                            : "Voice Chat"}
                    </h1>
                    <div class="flex items-center gap-1.5">
                        <span class="relative flex h-1.5 w-1.5">
                            <span
                                class="animate-ping absolute inline-flex h-full w-full rounded-full bg-green-400/50 opacity-75"
                            ></span>
                            <span
                                class="relative inline-flex rounded-full h-1.5 w-1.5 bg-green-500"
                            ></span>
                        </span>
                        <span class="text-xs text-brand-text/40">
                            {chatMode === "text"
                                ? "Powered by Gemini"
                                : "Powered by ElevenLabs"}
                        </span>
                    </div>
                </div>
            </div>

            <div class="flex items-center gap-2">
                <!-- Text/Voice Toggle -->
                <div
                    class="flex items-center bg-brand-text/5 rounded-xl p-1 border border-brand-text/10"
                >
                    <button
                        onclick={() => (chatMode = "text")}
                        class="flex items-center gap-1.5 px-3 py-1.5 rounded-lg text-sm font-medium transition-all cursor-pointer
                            {chatMode === 'text'
                            ? 'bg-brand-accent text-black shadow-sm'
                            : 'text-brand-text/50 hover:text-brand-text/70'}"
                    >
                        <Icon icon="mdi:keyboard" class="text-base" />
                        <span class="hidden sm:inline">Text</span>
                    </button>
                    <button
                        onclick={() => (chatMode = "voice")}
                        class="flex items-center gap-1.5 px-3 py-1.5 rounded-lg text-sm font-medium transition-all cursor-pointer
                            {chatMode === 'voice'
                            ? 'bg-brand-accent text-black shadow-sm'
                            : 'text-brand-text/50 hover:text-brand-text/70'}"
                    >
                        <Icon icon="mdi:microphone" class="text-base" />
                        <span class="hidden sm:inline">Voice</span>
                    </button>
                </div>

                {#if chatMode === "text"}
                    <button
                        onclick={handleClear}
                        class="p-2 rounded-lg hover:bg-brand-text/5 text-brand-text/30 hover:text-brand-text/60 transition"
                        title="Clear chat history"
                    >
                        <Icon
                            icon="mdi:delete-sweep-outline"
                            class="text-xl"
                        />
                    </button>
                {/if}
            </div>
        </div>

        <!-- Content Area -->
        {#if chatMode === "text"}
            <!-- Text Chat Messages -->
            <div
                bind:this={chatContainer}
                class="flex-1 overflow-y-auto space-y-5 mb-4 px-2 scrollbar-thin scrollbar-thumb-brand-text/10 scrollbar-track-transparent"
            >
                {#each messages as msg}
                    <div
                        class="flex w-full {msg.role === 'user'
                            ? 'justify-end'
                            : 'justify-start'}"
                    >
                        <div class="flex flex-col max-w-[80%]">
                            <span
                                class="text-[10px] uppercase tracking-wider text-brand-text/30 mb-1 px-1 {msg.role ===
                                'user'
                                    ? 'text-right'
                                    : 'text-left'}"
                                >{msg.role === "user"
                                    ? "You"
                                    : "LearningBuddy AI"}</span
                            >
                            <div
                                class="px-4 py-3 rounded-2xl text-sm {msg.role ===
                                'user'
                                    ? 'bg-brand-accent text-black rounded-tr-sm'
                                    : 'bg-brand-surface/80 text-brand-text border border-brand-text/10 rounded-tl-sm'}"
                            >
                                {#if msg.content}
                                    <div
                                        class="prose prose-sm dark:prose-invert max-w-none [&>*:first-child]:mt-0 [&>*:last-child]:mb-0"
                                    >
                                        {@html marked.parse(msg.content)}
                                    </div>
                                {:else}
                                    <span
                                        class="inline-flex gap-1 items-center text-brand-text/40"
                                    >
                                        <span
                                            class="w-1.5 h-1.5 bg-brand-accent rounded-full animate-bounce"
                                        ></span>
                                        <span
                                            class="w-1.5 h-1.5 bg-brand-accent rounded-full animate-bounce"
                                            style="animation-delay: 0.1s"
                                        ></span>
                                        <span
                                            class="w-1.5 h-1.5 bg-brand-accent rounded-full animate-bounce"
                                            style="animation-delay: 0.2s"
                                        ></span>
                                    </span>
                                {/if}
                            </div>
                        </div>
                    </div>
                {/each}
            </div>

            <!-- Text Input -->
            <form
                onsubmit={(e) => {
                    e.preventDefault();
                    send();
                }}
                class="flex gap-3 bg-brand-surface/80 backdrop-blur p-2 rounded-2xl border border-brand-text/10 focus-within:border-brand-accent/50 transition-colors flex-shrink-0"
            >
                <input
                    type="text"
                    bind:value={input}
                    placeholder="Ask a question about this source..."
                    class="flex-1 bg-transparent px-4 py-3 text-brand-text placeholder-brand-text/30 outline-none text-sm"
                    disabled={sending}
                />
                <button
                    type="submit"
                    class="bg-brand-accent text-black font-bold px-5 py-2.5 rounded-xl hover:scale-[1.02] transition-transform shadow-lg shadow-brand-accent/20 disabled:opacity-40 disabled:cursor-not-allowed disabled:hover:scale-100 cursor-pointer flex items-center gap-2 text-sm"
                    disabled={!input.trim() || sending}
                >
                    {#if sending}
                        <Icon
                            icon="mdi:loading"
                            class="animate-spin text-lg"
                        />
                    {:else}
                        <span>Send</span>
                        <Icon icon="mdi:send" class="text-sm" />
                    {/if}
                </button>
            </form>
        {:else}
            <!-- Voice Chat -->
            <div
                class="flex-1 flex flex-col bg-brand-surface/30 rounded-2xl border border-brand-text/10 overflow-hidden"
            >
                <VoiceChat {sourceId} />
            </div>
        {/if}
    </div>
{/if}

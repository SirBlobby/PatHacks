<script lang="ts">
    import { page } from "$app/stores";

    let input = $state("");
    let messages = $state([
        {
            role: "assistant",
            content:
                "Hello! I am ready to answer questions about this lecture. What would you like to know?",
        },
    ]);
    let chatContainer: HTMLDivElement;

    function scrollToBottom() {
        setTimeout(() => {
            if (chatContainer)
                chatContainer.scrollTop = chatContainer.scrollHeight;
        }, 0);
    }

    function send() {
        if (!input.trim()) return;

        messages = [...messages, { role: "user", content: input }];
        const currentInput = input;
        input = "";
        scrollToBottom();

        // Simulate AI response
        setTimeout(() => {
            messages = [
                ...messages,
                {
                    role: "assistant",
                    content: `That's a great question about "${currentInput}". Based on the transcript at [00:15:30], the professor mentioned that...`,
                },
            ];
            scrollToBottom();
        }, 1000);
    }
</script>

<div class="flex flex-col h-[calc(100vh-6rem)] max-w-4xl mx-auto w-full">
    <div
        class="flex items-center gap-4 mb-4 pb-4 border-b border-gray-800 bg-brand-bg/95 backdrop-blur z-10 sticky top-0"
    >
        <a
            href="/panel/recordings/{$page.params.id}"
            class="text-gray-400 hover:text-white transition flex items-center gap-2"
        >
            <span class="text-xl">&larr;</span> Back
        </a>
        <div>
            <h1 class="text-2xl font-bold text-white">
                Chat with Lecture #{$page.params.id}
            </h1>
            <div class="text-xs text-green-500 flex items-center gap-1">
                <div
                    class="w-2 h-2 bg-green-500 rounded-full animate-pulse"
                ></div>
                AI Online
            </div>
        </div>
    </div>

    <div
        bind:this={chatContainer}
        class="flex-1 overflow-y-auto space-y-6 mb-6 p-6 bg-gray-900/30 rounded-2xl border border-gray-800 scrollbar-thin scrollbar-thumb-gray-800 scrollbar-track-transparent"
    >
        {#each messages as msg}
            <div
                class="flex {msg.role === 'user'
                    ? 'justify-end'
                    : 'justify-start'} animate-fade-in-up"
            >
                <div class="flex flex-col max-w-[80%]">
                    <span
                        class="text-xs text-gray-500 mb-1 ml-1 {msg.role ===
                        'user'
                            ? 'text-right'
                            : 'text-left'}"
                        >{msg.role === "user"
                            ? "You"
                            : "LearningBuddy AI"}</span
                    >
                    <div
                        class="p-4 rounded-2xl shadow-lg border border-transparent {msg.role ===
                        'user'
                            ? 'bg-brand-accent text-black rounded-tr-none'
                            : 'bg-gray-800/80 backdrop-blur-md text-white rounded-tl-none border-gray-700'}"
                    >
                        {msg.content}
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
        class="flex gap-4 bg-gray-900/50 p-2 rounded-2xl border border-gray-800 focus-within:border-brand-accent transition-colors shadow-2xl relative"
    >
        <input
            type="text"
            bind:value={input}
            placeholder="Ask a question about the lecture transcript..."
            class="flex-1 bg-transparent px-4 py-3 text-white placeholder-gray-500 outline-none"
        />
        <button
            type="submit"
            class="bg-brand-accent text-black font-bold px-6 py-2 rounded-xl hover:scale-105 transition-transform shadow-lg shadow-brand-accent/20 disabled:opacity-50 disabled:cursor-not-allowed"
            disabled={!input.trim()}
        >
            Send
        </button>
    </form>
</div>

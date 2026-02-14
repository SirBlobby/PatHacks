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
        class="flex items-center justify-between gap-4 mb-4 pb-4 border-b border-brand-text/10 bg-brand-surface/95 backdrop-blur-xl z-20 sticky top-0 px-2 py-3 rounded-b-xl"
    >
        <div class="flex items-center gap-3">
            <a
                href="/panel/recordings/{$page.params.id}"
                class="p-2 rounded-lg hover:bg-brand-text/5 text-brand-text/60 hover:text-brand-text transition group"
                aria-label="Back to Recording"
            >
                <svg
                    xmlns="http://www.w3.org/2000/svg"
                    width="24"
                    height="24"
                    viewBox="0 0 24 24"
                    fill="none"
                    stroke="currentColor"
                    stroke-width="2"
                    stroke-linecap="round"
                    stroke-linejoin="round"
                    class="group-hover:-translate-x-1 transition-transform"
                    ><path d="m15 18-6-6 6-6" /></svg
                >
            </a>

            <div
                class="w-10 h-10 rounded-full bg-brand-accent/10 border border-brand-accent/20 flex items-center justify-center text-brand-accent shrink-0"
            >
                <!-- Robot Icon -->
                <svg
                    xmlns="http://www.w3.org/2000/svg"
                    width="20"
                    height="20"
                    viewBox="0 0 24 24"
                    fill="none"
                    stroke="currentColor"
                    stroke-width="2"
                    stroke-linecap="round"
                    stroke-linejoin="round"
                    ><rect
                        width="18"
                        height="18"
                        x="3"
                        y="3"
                        rx="2"
                        ry="2"
                    /><line x1="9" x2="15" y1="9" y2="9" /><line
                        x1="9"
                        x2="15"
                        y1="15"
                        y2="15"
                    /></svg
                >
            </div>

            <div>
                <h1 class="text-lg font-bold text-brand-text leading-tight">
                    Chat with Lecture #{$page.params.id}
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
                        >AI Assistant Online</span
                    >
                </div>
            </div>
        </div>

        <button
            class="p-2 rounded-lg hover:bg-brand-text/5 text-brand-text/40 hover:text-brand-text transition"
            title="Options"
        >
            <svg
                xmlns="http://www.w3.org/2000/svg"
                width="20"
                height="20"
                viewBox="0 0 24 24"
                fill="none"
                stroke="currentColor"
                stroke-width="2"
                stroke-linecap="round"
                stroke-linejoin="round"
                ><circle cx="12" cy="12" r="1" /><circle
                    cx="12"
                    cy="5"
                    r="1"
                /><circle cx="12" cy="19" r="1" /></svg
            >
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
        class="flex gap-4 bg-brand-bg/50 backdrop-blur p-2 rounded-2xl border border-brand-text/10 focus-within:border-brand-accent transition-colors shadow-lg relative"
    >
        <input
            type="text"
            bind:value={input}
            placeholder="Ask a question about the lecture transcript..."
            class="flex-1 bg-transparent px-4 py-3 text-brand-text placeholder-brand-text/50 outline-none"
        />
        <button
            type="submit"
            class="bg-brand-accent text-black font-bold px-6 py-2 rounded-xl hover:scale-105 transition-transform shadow-lg shadow-brand-accent/20 disabled:opacity-50 disabled:cursor-not-allowed cursor-pointer"
            disabled={!input.trim()}
        >
            Send
        </button>
    </form>
</div>

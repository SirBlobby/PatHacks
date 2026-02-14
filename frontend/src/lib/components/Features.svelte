<script lang="ts">
	const features = [
		{
			title: "Smart Recording",
			desc: "Capture every word of your lectures with crystal clear audio recording and intelligent noise handling.",
		},
		{
			title: "Instant Transcription",
			desc: "Convert speech to text in real-time with high accuracy, supporting multiple languages and dialects.",
		},
		{
			title: "AI Summaries",
			desc: "Get concise summaries and key takeaways automatically generated, organizing complex topics effortlessly.",
		},
		{
			title: "Interactive Q&A",
			desc: "Ask questions about your lectures and get instant answers from your AI buddy, backed by vector search.",
		},
	];
</script>

<section class="py-20 relative z-10 w-full mb-12">
	<h2
		class="text-4xl md:text-5xl font-black text-center mb-16 text-brand-text"
	>
		Key Features
	</h2>
	<div class="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-8 px-4">
		{#each features as feature}
			<div
				class="p-8 bg-brand-bg/50 backdrop-blur-md border-2 border-brand-text/10 rounded-2xl shadow-lg hover:border-brand-accent hover:shadow-brand-accent/20 transition-all duration-300 group"
			>
				<h3
					class="text-xl font-bold mb-3 text-brand-text group-hover:text-brand-accent transition-colors"
				>
					{feature.title}
				</h3>
				<p class="text-brand-text/70 leading-relaxed">{feature.desc}</p>
			</div>
		{/each}
	</div>
</section>

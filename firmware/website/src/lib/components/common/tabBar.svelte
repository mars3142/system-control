<script lang="ts">
	interface Props {
		children?: import('svelte').Snippet;
	}

	let { children }: Props = $props();
</script>

<div class="flex justify-center mb-6">
	<div
		class="inline-flex bg-card rounded-lg p-1 border border-border shadow-sm"
	>
		{@render children?.()}
	</div>
</div>

<script lang="ts">
	interface Props {
		active?: boolean;
		label?: string;
		onClick: () => void;
	}

	let { active = false, label = "", onClick }: Props = $props();
</script>

<button
	class="px-6 py-2 rounded-md text-sm font-medium transition-colors {active
		? 'bg-primary text-white shadow-sm'
		: 'text-muted-foreground hover:text-foreground'}"
	onclick={onClick}
>
	{label}
</button>

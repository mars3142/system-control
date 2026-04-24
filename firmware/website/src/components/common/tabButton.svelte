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
		: 'text-text-muted hover:text-text'}"
	onclick={onClick}
>
	{label}
</button>

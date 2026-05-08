<script lang="ts">
	let {
		active = false,
		icon,
		label,
		onClick
	}: { active?: boolean; icon: string; label: string; onClick: () => void } = $props();
</script>

<button
	class="flex-1 flex flex-col items-center justify-center p-4 rounded-md border transition-all {active
		? 'bg-primary text-white border-primary shadow-md'
		: 'bg-card text-text border-border hover:bg-accent hover:text-text'}"
	onclick={onClick}
>
	<span class="text-2xl mb-1">{icon}</span>
	<span class="text-sm font-medium">{label}</span>
</button>

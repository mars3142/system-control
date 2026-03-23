<script lang="ts">
	const { label, icon, ariaLabel, onClick } = $props<{
		label: string;
		icon?: string;
		ariaLabel: string;
		onClick?: () => void;
	}>();
</script>

<button
	aria-label={ariaLabel}
	class="flex items-center justify-center cursor-pointer p-2 gap-2 bg-card rounded-lg text-text-muted text-sm transition-all border-solid border-border border-x border-y hover:border-success min-w-28"
	onclick={onClick}
>
	{#if icon}
		<span class="text-xl" id="icon">{icon}</span>
	{/if}
	<span class="label" id="label">{label}</span>
</button>

<script lang="ts">
	export let active: boolean = false;
	export let label: string = "";
	export let onClick: () => void;
</script>

<button
	class="px-6 py-2 rounded-md text-sm font-medium transition-colors {active
		? 'bg-primary text-white shadow-sm'
		: 'text-muted-foreground hover:text-foreground'}"
	on:click={onClick}
>
	{label}
</button>

<script lang="ts">
	interface Props {
		name: string;
		start: number;
		leds: number;
		onDelete: () => void;
	}

	let { name = $bindable(), start = $bindable(), leds = $bindable(), onDelete }: Props = $props();
</script>

<tr class="group">
	<td class="px-3 py-2 text-sm bg-bg-secondary group-hover:bg-accent border-y border-l border-border">
		<input
			type="text"
			bind:value={name}
			class="w-full px-1 py-0.5 text-sm rounded border border-border bg-input focus:outline-none focus:border-primary"
		/>
	</td>
	<td class="px-3 py-2 text-sm text-center bg-bg-secondary group-hover:bg-accent border-y border-border">
		<input
			type="number"
			min="0"
			bind:value={start}
			class="w-16 px-1 py-0.5 text-sm text-right rounded border border-border bg-input focus:outline-none focus:border-primary no-spinner"
		/>
	</td>
	<td class="px-3 py-2 text-sm text-center bg-bg-secondary group-hover:bg-accent border-y border-border">
		<input
			type="number"
			min="1"
			bind:value={leds}
			class="w-16 px-1 py-0.5 text-sm text-right rounded border border-border bg-input focus:outline-none focus:border-primary no-spinner"
		/>
	</td>
	<td class="px-1 py-2 text-sm text-center bg-bg-secondary group-hover:bg-accent border-y border-r border-border">
		<button
			onclick={onDelete}
			aria-label="delete"
			class="flex items-center justify-center cursor-pointer px-1 py-1 bg-card rounded-lg text-text-muted text-sm transition-all border-solid border-border border-x border-y hover:border-primary"
		>🗑️</button>
	</td>
</tr>

<style>
	:global(input.no-spinner::-webkit-inner-spin-button),
	:global(input.no-spinner::-webkit-outer-spin-button) {
		-webkit-appearance: none;
		margin: 0;
	}
	:global(input.no-spinner) {
		-moz-appearance: textfield;
	}
</style>

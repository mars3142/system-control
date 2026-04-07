<script lang="ts">
	interface Props {
		time: string;
		r: number;
		g: number;
		b: number;
		brightness: number;
	}

	let { time, r = $bindable(), g = $bindable(), b = $bindable(), brightness = $bindable() }: Props = $props();

	function clamp(v: number): number {
		return Math.min(255, Math.max(0, Math.round(v)));
	}
</script>

<tr class="group">
	<td class="px-3 py-2 text-sm bg-bg-secondary group-hover:bg-accent border-y border-l border-border">{time}</td>
	<td class="px-2 py-2 text-sm bg-bg-secondary group-hover:bg-accent border-y border-border">
		<div class="h-6 rounded" style="background: rgb({r},{g},{b});"></div>
	</td>
	<td class="px-3 py-2 text-sm text-center bg-bg-secondary group-hover:bg-accent border-y border-border">
		<input
			type="number" min="0" max="255"
			bind:value={r}
			oninput={() => r = clamp(r)}
			class="w-14 px-1 py-0.5 text-sm text-center rounded border border-border bg-input focus:outline-none focus:border-primary no-spinner"
		/>
	</td>
	<td class="px-3 py-2 text-sm text-center bg-bg-secondary group-hover:bg-accent border-y border-border">
		<input
			type="number" min="0" max="255"
			bind:value={g}
			oninput={() => g = clamp(g)}
			class="w-14 px-1 py-0.5 text-sm text-center rounded border border-border bg-input focus:outline-none focus:border-primary no-spinner"
		/>
	</td>
	<td class="px-3 py-2 text-sm text-center bg-bg-secondary group-hover:bg-accent border-y border-border">
		<input
			type="number" min="0" max="255"
			bind:value={b}
			oninput={() => b = clamp(b)}
			class="w-14 px-1 py-0.5 text-sm text-center rounded border border-border bg-input focus:outline-none focus:border-primary no-spinner"
		/>
	</td>
	<td class="px-3 py-2 text-sm text-center bg-bg-secondary group-hover:bg-accent border-y border-r border-border">
		<input
			type="number" min="0" max="255"
			bind:value={brightness}
			oninput={() => brightness = clamp(brightness)}
			class="w-14 px-1 py-0.5 text-sm text-center rounded border border-border bg-input focus:outline-none focus:border-primary no-spinner"
		/>
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

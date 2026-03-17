<script lang="ts">
	import { type ControlState, controlStore, createDefaultControlState } from '../../stores/controlStore';
	import LightControl from './lightControl.svelte';
	import ModeControl from './modeControl.svelte';
	import StatusDisplay from './statusDisplay.svelte';

	let state = $state<ControlState>(createDefaultControlState());
	$effect(() => {
		return controlStore.subscribe((value) => {
			if (value) state = value;
		});
	});

	function setLight(on: boolean) {
		controlStore.setLight({ on });
	}

	function setMode(mode: string) {
		controlStore.setMode({ mode });
	}

	function setSchema(schema: string) {
		controlStore.setSchema({ schema });
	}

	// Hilfsfunktion für CSS-Farbe
	function colorToCss(color: any): string {
		if (!color) return '#000';
		if (typeof color === 'string') return color;
		if (
			typeof color === 'object' &&
			color.r !== undefined &&
			color.g !== undefined &&
			color.b !== undefined
		) {
			return `rgb(${color.r},${color.g},${color.b})`;
		}
		return '#000';
	}
</script>

<div class="space-y-6">
	<LightControl lightOn={state.on} onchange={(e) => setLight(e.detail)} />

	<ModeControl
		bind:activeSchema={state.schema}
		bind:mode={state.mode}
		onchangeSchema={(e) => setSchema(e.detail)}
		onchangeMode={(e) => setMode(e.detail)}
	/>

	<StatusDisplay
		clock={state.clock}
		color={colorToCss(state.color)}
		mode={state.mode}
	/>
</div>
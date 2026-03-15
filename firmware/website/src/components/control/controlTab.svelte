<script lang="ts">
	import { controlStore, type ControlState } from "../../stores/controlStore";
	import LightControl from "./lightControl.svelte";
	import ModeControl from "./modeControl.svelte";
	import StatusDisplay from "./statusDisplay.svelte";

	let state = $state<ControlState>({
		on: false,
		mode: 'day',
		schema: 'schema_01.csv',
		color: { r: 0, g: 0, b: 0 },
		clock: '00:00'
	});
	$effect(() => {
		return controlStore.subscribe((value) => {
			if (value) state = value;
		});
	});


	function setMode(mode: string) {
		controlStore.setState({ mode });
	}

	function handleSchemaChange(schema: string) {
		controlStore.setState({ schema });
	}

	// Hilfsfunktion für CSS-Farbe
	function colorToCss(color: any): string {
		if (!color) return "#000";
		if (typeof color === "string") return color;
		if (
			typeof color === "object" &&
			color.r !== undefined &&
			color.g !== undefined &&
			color.b !== undefined
		) {
			return `rgb(${color.r},${color.g},${color.b})`;
		}
		return "#000";
	}
</script>

<div class="space-y-6">
	<LightControl lightOn={state.on} />

	<ModeControl
		bind:activeSchema={state.schema}
		bind:mode={state.mode}
		onchangeSchema={(e) => handleSchemaChange(e.detail)}
		onchangeMode={(e) => setMode(e.detail)}
	/>

	<StatusDisplay
		clock={state.clock}
		color={colorToCss(state.color)}
		mode={state.mode}
	/>
</div>
<script lang="ts">
	import LightControl from './lightControl.svelte';
	import ModeControl from './modeControl.svelte';
	import StatusDisplay from './statusDisplay.svelte';
</script>

<div class="space-y-6">
	<LightControl />

	<ModeControl />

	<StatusDisplay />
</div>
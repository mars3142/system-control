<script lang="ts">
	let {
		checked = $bindable(false),
		disabled = false,
		label,
		onchange,
	} = $props<{
		checked?: boolean;
		disabled?: boolean;
		label?: string;
		onchange?: (checked: boolean) => void;
	}>();

	function toggle() {
		if (!disabled) {
			checked = !checked;
			onchange?.(checked);
		}
	}
</script>

<div class="flex items-center justify-between">
	{#if label}
		<span class="font-medium">{label}</span>
	{/if}
	<!-- svelte-ignore a11y_click_events_have_key_events -->
	<!-- svelte-ignore a11y_no_static_element_interactions -->
	<div
		class="relative w-11 h-6 rounded-full cursor-pointer transition-colors duration-200 ease-in-out {checked
			? 'bg-primary'
			: 'bg-gray-200 dark:bg-gray-700'} {disabled
			? 'opacity-50 cursor-not-allowed'
			: ''}"
		onclick={toggle}
	>
		<div
			class="absolute top-[2px] left-[2px] bg-white border-gray-300 border rounded-full h-5 w-5 transition-transform duration-200 ease-in-out {checked
				? 'translate-x-full border-white'
				: 'translate-x-0'}"
		></div>
	</div>
</div>

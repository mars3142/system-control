<script lang="ts">
	import Card from '../common/card.svelte';
	import Button from '../common/button.svelte';
	import { t } from '../../i18n/store';
</script>

<Card title="wled.config.title">
	<p class="text-sm -mt-3 mb-4 text-text-muted">{$t("wled.config.desc")}</p>

	<div class="flex justify-center items-center">
		<div class="flex-1">Segmente</div>
		<Button label={$t("wled.segment.add")} ariaLabel={$t("wled.segment.add")}></Button>
	</div>
</Card>

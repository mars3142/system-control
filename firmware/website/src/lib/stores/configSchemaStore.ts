import { writable } from 'svelte/store';
import { baseUrl } from '../utils/apiClient';
import { createLogger } from '../utils/logger';

const log = createLogger('configSchemaStore');

export interface SchemaRow {
  r: number;
  g: number;
  b: number;
  w: number;
  brightness: number;
  saturation: number;
}

function parseCSV(csv: string): SchemaRow[] {
  return csv
    .trim()
    .split('\n')
    .filter((line) => line.trim().length > 0)
    .map((line) => {
      const [r, g, b, w, brightness, saturation] = line.split(',').map(Number);
      return { r, g, b, w, brightness, saturation };
    });
}

function toCSV(rows: SchemaRow[]): string {
  return rows.map((row) => `${row.r},${row.g},${row.b},${row.w},${row.brightness},${row.saturation}`).join('\n');
}

function createSchemaStore() {
  const { subscribe, set } = writable<SchemaRow[]>([]);

  async function fetchSchema(filename: string): Promise<void> {
    log.debug('Loading schema', { filename });
    const res = await fetch(`${baseUrl}/api/schema/${filename}`);
    if (!res.ok) throw new Error(`Failed to load schema: ${res.status}`);
    const text = await res.text();
    set(parseCSV(text));
    log.debug('Schema loaded', { filename });
  }

  async function saveSchema(filename: string, rows: SchemaRow[]): Promise<void> {
    log.debug('Saving schema', { filename });
    const res = await fetch(`${baseUrl}/api/schema/${filename}`, {
      method: 'POST',
      headers: { 'Content-Type': 'text/csv' },
      body: toCSV(rows)
    });
    if (!res.ok) throw new Error(`Failed to save schema: ${res.status}`);
    log.debug('Schema saved', { filename });
  }

  return { subscribe, fetchSchema, saveSchema };
}

export const schemaStore = createSchemaStore();

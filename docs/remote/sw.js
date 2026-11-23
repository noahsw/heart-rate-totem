// Service worker for the phone remote: keeps a copy of the page on the phone
// so it opens with no signal (festivals often have none).
//
// Network first: with signal, the phone always gets the latest page and
// refreshes its saved copy. With no signal, or a weak one that doesn't answer
// within a few seconds, it shows the saved copy. The page needs nothing else
// from the internet: it talks to the totem over Bluetooth.

const CACHE = 'totem-remote';
const PAGE = './';
const TIMEOUT_MS = 3000;

self.addEventListener('install', (event) => {
  event.waitUntil(caches.open(CACHE).then((c) => c.add(PAGE)).then(() => self.skipWaiting()));
});

self.addEventListener('activate', (event) => {
  event.waitUntil(self.clients.claim());
});

self.addEventListener('fetch', (event) => {
  const req = event.request;
  if (req.method !== 'GET' || req.mode !== 'navigate') return;
  event.respondWith(fromNetworkOrCache());
});

async function fromNetworkOrCache() {
  const cache = await caches.open(CACHE);
  const saved = await cache.match(PAGE);
  try {
    const timeout = saved ? AbortSignal.timeout(TIMEOUT_MS) : undefined;
    const fresh = await fetch(PAGE, { cache: 'no-cache', signal: timeout });
    if (fresh.ok) await cache.put(PAGE, fresh.clone());
    return fresh;
  } catch (err) {
    if (saved) return saved;
    throw err;
  }
}

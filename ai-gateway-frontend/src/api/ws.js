const WS_URL = import.meta.env.VITE_WS_URL || `ws://${window.location.host}/ws`

let ws = null
let reconnectTimer = null
let reconnectAttempts = 0
const maxReconnectAttempts = 5

const callbacks = {
  onMessage: [],
  onConnect: [],
  onDisconnect: []
}

function handleMessage(event) {
  try {
    const data = JSON.parse(event.data)
    callbacks.onMessage.forEach(cb => cb(data))
  } catch (e) {
    console.error('WS parse error', e)
  }
}

export function connect(token) {
  if (ws && ws.readyState === WebSocket.OPEN) return

  ws = new WebSocket(`${WS_URL}/${token}`)

  ws.onopen = () => {
    console.log('WebSocket connected')
    reconnectAttempts = 0
    clearTimeout(reconnectTimer)
    callbacks.onConnect.forEach(cb => cb())
    // Auth
    ws.send(JSON.stringify({ type: 'auth' }))
  }

  ws.onmessage = handleMessage

  ws.onclose = () => {
    console.log('WebSocket disconnected')
    callbacks.onDisconnect.forEach(cb => cb())
    scheduleReconnect(token)
  }

  ws.onerror = (e) => console.error('WebSocket error', e)
}

function scheduleReconnect(token) {
  if (reconnectAttempts >= maxReconnectAttempts) return
  const delay = Math.min(1000 * Math.pow(2, reconnectAttempts), 30000)
  reconnectAttempts++
  reconnectTimer = setTimeout(() => connect(token), delay)
}

export function send(data) {
  if (ws && ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify(data))
  }
}

export function disconnect() {
  clearTimeout(reconnectTimer)
  reconnectAttempts = maxReconnectAttempts
  if (ws) {
    ws.close()
    ws = null
  }
}

export function onMessage(cb) { callbacks.onMessage.push(cb) }
export function onConnect(cb) { callbacks.onConnect.push(cb) }
export function onDisconnect(cb) { callbacks.onDisconnect.push(cb) }

export function sendChat(model, messages, stream = true) {
  send({ type: 'chat', model, messages, stream })
}

export default { connect, disconnect, send, onMessage, onConnect, onDisconnect, sendChat }
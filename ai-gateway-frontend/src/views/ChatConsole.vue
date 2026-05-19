<script setup>
import { ref, onMounted, onUnmounted } from 'vue'
import { useAuthStore } from '@/stores/auth'
import ws from '@/api/ws'
import ChatMessage from '@/components/ChatMessage.vue'

const auth = useAuthStore()
const selectedModel = ref('deepseek')
const inputMessage = ref('')
const messages = ref([])
const streaming = ref(false)

const models = [
  { label: 'DeepSeek', value: 'deepseek' },
  { label: 'MiniMax', value: 'minimax' },
  { label: 'OpenAI', value: 'openai' }
]

onMounted(() => {
  ws.connect(auth.token)
  ws.onMessage(handleWsMessage)
  // Load history from localStorage
  const history = localStorage.getItem('chat_history')
  if (history) messages.value = JSON.parse(history)
})

onUnmounted(() => {
  ws.disconnect()
})

function handleWsMessage(data) {
  if (data.type === 'stream') {
    const last = messages.value[messages.value.length - 1]
    if (last && last.role === 'assistant') {
      last.content += data.content || ''
    } else {
      messages.value.push({ role: 'assistant', content: data.content || '', model: data.model })
    }
  } else if (data.type === 'done') {
    streaming.value = false
    saveHistory()
  }
}

async function sendMessage() {
  if (!inputMessage.value.trim() || streaming.value) return

  const msg = { role: 'user', content: inputMessage.value }
  messages.value.push(msg)
  inputMessage.value = ''
  streaming.value = true

  // Add placeholder for assistant
  messages.value.push({ role: 'assistant', content: '', model: selectedModel.value })

  ws.sendChat(selectedModel.value, messages.value.slice(0, -1).map(m => ({ role: m.role, content: m.content })), true)
}

function saveHistory() {
  const toSave = messages.value.filter(m => m.role === 'user' || m.role === 'assistant')
  localStorage.setItem('chat_history', JSON.stringify(toSave))
}

function clearChat() {
  messages.value = []
  localStorage.removeItem('chat_history')
}
</script>

<template>
  <div class="chat-console">
    <h2>AI 对话控制台</h2>
    <el-card class="chat-card">
      <div class="model-selector">
        <el-select v-model="selectedModel" placeholder="选择模型" style="width:200px">
          <el-option v-for="m in models" :key="m.value" :label="m.label" :value="m.value" />
        </el-select>
        <el-button @click="clearChat" style="margin-left:8px">清空对话</el-button>
      </div>

      <div class="messages">
        <ChatMessage v-for="(msg, i) in messages" :key="i" :message="msg" />
      </div>

      <div class="input-area">
        <el-input v-model="inputMessage" type="textarea" :rows="3" placeholder="输入消息..."
          @keydown.ctrl.enter="sendMessage" />
        <el-button type="primary" :loading="streaming" @click="sendMessage" style="margin-top:8px">
          {{ streaming ? '生成中...' : '发送 (Ctrl+Enter)' }}
        </el-button>
      </div>
    </el-card>
  </div>
</template>

<style scoped>
.chat-console { padding: 0 8px; }
.chat-card { display: flex; flex-direction: column; height: calc(100vh - 120px); }
.model-selector { display: flex; align-items: center; margin-bottom: 16px; }
.messages { flex: 1; overflow-y: auto; padding: 8px 0; }
.input-area { border-top: 1px solid #e6e6e6; padding-top: 12px; }
</style>
<script setup>
defineProps({
  message: Object
})
</script>

<template>
  <div class="chat-message" :class="message.role">
    <div class="avatar">{{ message.role === 'user' ? 'U' : 'AI' }}</div>
    <div class="bubble">
      <div class="meta" v-if="message.role === 'assistant'">
        <el-tag size="small">{{ message.model }}</el-tag>
      </div>
      <div class="text">{{ message.content }}</div>
    </div>
  </div>
</template>

<style scoped>
.chat-message { display: flex; margin-bottom: 16px; align-items: flex-start; gap: 8px; }
.chat-message.user { flex-direction: row-reverse; }
.avatar { width: 36px; height: 36px; border-radius: 50%; background: #409EFF; color: #fff; display: flex; align-items: center; justify-content: center; font-weight: bold; flex-shrink: 0; }
.chat-message.assistant .avatar { background: #67C23A; }
.bubble { max-width: 70%; background: #f0f0f0; border-radius: 12px; padding: 10px 14px; }
.chat-message.user .bubble { background: #409EFF; color: #fff; }
.meta { margin-bottom: 4px; }
.text { white-space: pre-wrap; word-break: break-word; }
</style>
<script setup>
defineProps({
  device: Object
})

const emit = defineEmits(['bind', 'command'])

const commands = [
  { label: '唤醒 (WOL)', value: 'wol' },
  { label: 'Ping', value: 'ping' },
  { label: '重启', value: 'reboot' },
]

function getStatusColor(status) {
  return status === 'online' ? '#67C23A' : '#909399'
}
</script>

<template>
  <el-card class="device-card" shadow="hover">
    <div class="device-header">
      <span class="device-name">{{ device.name || '未命名设备' }}</span>
      <el-tag size="small" :color="getStatusColor(device.status)" effect="dark">
        {{ device.status === 'online' ? '在线' : '离线' }}
      </el-tag>
    </div>
    <div class="device-info">
      <div><strong>IP:</strong> {{ device.ip }}</div>
      <div><strong>MAC:</strong> {{ device.mac }}</div>
      <div><strong>类型:</strong> {{ device.device_type || '未知' }}</div>
      <div><strong>最后出现:</strong> {{ device.last_seen ? new Date(device.last_seen * 1000).toLocaleString() : '未知' }}</div>
    </div>
    <div class="device-actions">
      <el-button size="small" type="primary" plain @click="$emit('bind', device)">编辑</el-button>
      <el-dropdown trigger="click" @command="(cmd) => $emit('command', cmd)">
        <el-button size="small" type="info" plain>发送指令</el-button>
        <template #dropdown>
          <el-dropdown-menu v-for="cmd in commands" :key="cmd.value">
            <el-dropdown-item :command="cmd.value">{{ cmd.label }}</el-dropdown-item>
          </el-dropdown-menu>
        </template>
      </el-dropdown>
    </div>
  </el-card>
</template>

<style scoped>
.device-card { margin-bottom: 12px; }
.device-header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 8px; }
.device-name { font-weight: bold; font-size: 15px; }
.device-info { font-size: 13px; color: #666; line-height: 1.8; }
.device-actions { display: flex; gap: 8px; margin-top: 10px; }
</style>
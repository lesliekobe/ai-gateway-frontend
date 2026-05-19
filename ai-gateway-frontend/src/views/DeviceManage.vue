<script setup>
import { ref, onMounted } from 'vue'
import api from '@/api/device'
import { ElMessage } from 'element-plus'
import DeviceCard from '@/components/DeviceCard.vue'

const devices = ref([])
const loading = ref(false)
const dialogVisible = ref(false)
const form = ref({ name: '', ip: '', mac: '', device_type: '' })

onMounted(async () => { await fetchDevices() })

async function fetchDevices() {
  loading.value = true
  try {
    devices.value = await api.getDevices()
  } finally {
    loading.value = false
  }
}

async function handleScan() {
  loading.value = true
  ElMessage.info('正在扫描局域网...')
  try {
    await api.scanDevices()
    await fetchDevices()
    ElMessage.success('扫描完成')
  } catch (e) {
    ElMessage.error('扫描失败')
  } finally {
    loading.value = false
  }
}

async function handleBind(device) {
  dialogVisible.value = true
  form.value = { ...device }
}

async function handleSendCommand(device, cmd) {
  try {
    await api.sendCommand(device.id, { command: cmd })
    ElMessage.success(`指令 "${cmd}" 已发送`)
  } catch (e) {
    ElMessage.error('发送失败')
  }
}

async function handleSaveBind() {
  try {
    await api.bindDevice(form.value.id, form.value)
    dialogVisible.value = false
    await fetchDevices()
    ElMessage.success('绑定成功')
  } catch (e) {
    ElMessage.error('绑定失败')
  }
}
</script>

<template>
  <div class="device-manage">
    <h2>设备管理</h2>
    <el-card>
      <div style="margin-bottom:12px">
        <el-button type="primary" :loading="loading" @click="handleScan">扫描局域网设备</el-button>
      </div>
      <el-row :gutter="12">
        <el-col :span="6" v-for="device in devices" :key="device.id">
          <DeviceCard :device="device" @bind="handleBind" @command="(cmd) => handleSendCommand(device, cmd)" />
        </el-col>
      </el-row>
      <el-empty v-if="!loading && devices.length === 0" description="暂无设备，请点击扫描" />
    </el-card>

    <el-dialog v-model="dialogVisible" title="设备绑定" width="400px">
      <el-form :model="form" label-width="80px">
        <el-form-item label="名称"><el-input v-model="form.name" /></el-form-item>
        <el-form-item label="IP"><el-input v-model="form.ip" readonly /></el-form-item>
        <el-form-item label="MAC"><el-input v-model="form.mac" readonly /></el-form-item>
        <el-form-item label="类型"><el-input v-model="form.device_type" /></el-form-item>
      </el-form>
      <template #footer>
        <el-button @click="dialogVisible = false">取消</el-button>
        <el-button type="primary" @click="handleSaveBind">保存</el-button>
      </template>
    </el-dialog>
  </div>
</template>
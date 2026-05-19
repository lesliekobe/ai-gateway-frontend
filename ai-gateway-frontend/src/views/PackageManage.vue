<script setup>
import { ref, onMounted } from 'vue'
import api from '@/api/index'
import { ElMessage } from 'element-plus'

const packages = ref([])
const dialogVisible = ref(false)
const form = ref({ name: '', hourly_limit: 100, daily_limit: 1000, max_concurrent: 2, allowed_models: [], price: 0 })

onMounted(async () => {
  const res = await api.get('/package')
  packages.value = res
})

function openDialog(pkg = null) {
  form.value = pkg ? { ...pkg, allowed_models: pkg.allowed_models || [] } : { name: '', hourly_limit: 100, daily_limit: 1000, max_concurrent: 2, allowed_models: [], price: 0 }
  dialogVisible.value = true
}

async function handleSave() {
  try {
    if (form.value.id) {
      await api.put(`/package/${form.value.id}`, form.value)
    } else {
      await api.post('/package', form.value)
    }
    dialogVisible.value = false
    const res = await api.get('/package')
    packages.value = res
    ElMessage.success('保存成功')
  } catch (e) {
    ElMessage.error('保存失败')
  }
}
</script>

<template>
  <div class="package-manage">
    <h2>套餐管理</h2>
    <el-card>
      <div style="margin-bottom:12px">
        <el-button type="primary" @click="openDialog()">新建套餐</el-button>
      </div>
      <el-table :data="packages" style="width:100%">
        <el-table-column prop="name" label="套餐名" width="150" />
        <el-table-column prop="hourly_limit" label="小时限制" width="100" />
        <el-table-column prop="daily_limit" label="日限制" width="100" />
        <el-table-column prop="max_concurrent" label="并发上限" width="100" />
        <el-table-column prop="allowed_models" label="可用模型" width="200">
          <template #default="{ row }">
            <el-tag v-for="m in (row.allowed_models || [])" :key="m" size="small" style="margin-right:4px">{{ m }}</el-tag>
          </template>
        </el-table-column>
        <el-table-column prop="price" label="价格" width="100">
          <template #default="{ row }">¥{{ row.price }}</template>
        </el-table-column>
        <el-table-column label="操作" width="120">
          <template #default="{ row }">
            <el-button size="small" @click="openDialog(row)">编辑</el-button>
          </template>
        </el-table-column>
      </el-table>
    </el-card>

    <el-dialog v-model="dialogVisible" :title="form.id ? '编辑套餐' : '新建套餐'" width="520px">
      <el-form :model="form" label-width="100px">
        <el-form-item label="套餐名"><el-input v-model="form.name" /></el-form-item>
        <el-form-item label="小时限制"><el-input-number v-model="form.hourly_limit" :min="1" /></el-form-item>
        <el-form-item label="日限制"><el-input-number v-model="form.daily_limit" :min="1" /></el-form-item>
        <el-form-item label="并发上限"><el-input-number v-model="form.max_concurrent" :min="1" /></el-form-item>
        <el-form-item label="可用模型">
          <el-select v-model="form.allowed_models" multiple placeholder="选择模型" style="width:100%">
            <el-option label="MiniMax" value="minimax" />
            <el-option label="DeepSeek" value="deepseek" />
            <el-option label="OpenAI" value="openai" />
          </el-select>
        </el-form-item>
        <el-form-item label="价格"><el-input-number v-model="form.price" :min="0" :precision="2" /></el-form-item>
      </el-form>
      <template #footer>
        <el-button @click="dialogVisible = false">取消</el-button>
        <el-button type="primary" @click="handleSave">保存</el-button>
      </template>
    </el-dialog>
  </div>
</template>
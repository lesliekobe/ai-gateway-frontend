<script setup>
import { ref, onMounted } from 'vue'
import { useUsersStore } from '@/stores/users'
import { ElMessage, ElMessageBox } from 'element-plus'

const users = useUsersStore()
const loading = ref(false)
const dialogVisible = ref(false)
const form = ref({ username: '', password: '', email: '', is_admin: false, package_id: null })

onMounted(() => users.fetchUsers())

async function handleSearch(query) {
  await users.fetchUsers({ search: query })
}

function openDialog(user = null) {
  form.value = user ? { ...user } : { username: '', password: '', email: '', is_admin: false, package_id: null }
  dialogVisible.value = true
}

async function handleSave() {
  try {
    if (form.value.id) {
      await users.updateUser(form.value.id, form.value)
    } else {
      await users.createUser(form.value)
    }
    dialogVisible.value = false
    await users.fetchUsers()
    ElMessage.success('保存成功')
  } catch (e) {
    ElMessage.error('保存失败')
  }
}

async function handleDelete(row) {
  await ElMessageBox.confirm('确定删除该用户？', '警告', { type: 'warning' })
  await users.deleteUser(row.id)
  await users.fetchUsers()
  ElMessage.success('已删除')
}

async function handleReset(row) {
  await ElMessageBox.confirm('确定重置该用户密码？', '警告', { type: 'warning' })
  await users.resetPassword(row.id)
  ElMessage.success('密码已重置为: 123456')
}
</script>

<template>
  <div class="user-manage">
    <h2>用户管理</h2>
    <el-card>
      <div class="toolbar">
        <el-input placeholder="搜索用户名" @search="handleSearch" style="width:240px" clearable />
        <el-button type="primary" @click="openDialog()">新建用户</el-button>
      </div>

      <el-table :data="users.users" v-loading="loading" style="width:100%">
        <el-table-column prop="username" label="用户名" width="150" />
        <el-table-column prop="email" label="邮箱" width="180" />
        <el-table-column prop="is_admin" label="管理员" width="100">
          <template #default="{ row }">
            <el-tag :type="row.is_admin ? 'danger' : 'success'">{{ row.is_admin ? '是' : '否' }}</el-tag>
          </template>
        </el-table-column>
        <el-table-column prop="is_active" label="状态" width="100">
          <template #default="{ row }">
            <el-tag :type="row.is_active ? 'success' : 'info'">{{ row.is_active ? '活跃' : '禁用' }}</el-tag>
          </template>
        </el-table-column>
        <el-table-column prop="package_name" label="套餐" width="120" />
        <el-table-column prop="created_at" label="注册时间" width="180">
          <template #default="{ row }">
            {{ new Date(row.created_at * 1000).toLocaleString() }}
          </template>
        </el-table-column>
        <el-table-column label="操作" width="240">
          <template #default="{ row }">
            <el-button size="small" @click="openDialog(row)">编辑</el-button>
            <el-button size="small" type="warning" @click="handleReset(row)">重置密码</el-button>
            <el-button size="small" type="danger" @click="handleDelete(row)">删除</el-button>
          </template>
        </el-table-column>
      </el-table>

      <el-pagination layout="prev, pager, next" :total="users.total"
        v-model:current-page="users.page" @current-change="users.fetchUsers()" style="margin-top:16px" />
    </el-card>

    <el-dialog v-model="dialogVisible" :title="form.id ? '编辑用户' : '新建用户'" width="480px">
      <el-form :model="form" label-width="100px">
        <el-form-item label="用户名"><el-input v-model="form.username" /></el-form-item>
        <el-form-item label="密码" v-if="!form.id"><el-input v-model="form.password" type="password" show-password /></el-form-item>
        <el-form-item label="邮箱"><el-input v-model="form.email" /></el-form-item>
        <el-form-item label="管理员">
          <el-switch v-model="form.is_admin" />
        </el-form-item>
      </el-form>
      <template #footer>
        <el-button @click="dialogVisible = false">取消</el-button>
        <el-button type="primary" @click="handleSave">保存</el-button>
      </template>
    </el-dialog>
  </div>
</template>

<style scoped>
.toolbar { display: flex; gap: 12px; margin-bottom: 16px; }
</style>
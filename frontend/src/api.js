import axios from 'axios';

const API_BASE_URL = 'http://localhost:8081/api';

class APIClient {
    constructor() {
        this.token = localStorage.getItem('token') || null;
        this.user = JSON.parse(localStorage.getItem('user') || 'null');
    }

    setAuthToken(token) {
        this.token = token;
        localStorage.setItem('token', token);
    }

    setUser(user) {
        this.user = user;
        localStorage.setItem('user', JSON.stringify(user));
    }

    clearAuth() {
        this.token = null;
        this.user = null;
        localStorage.removeItem('token');
        localStorage.removeItem('user');
    }

    getHeaders() {
        return {
            'Content-Type': 'application/json',
            ...(this.token && { 'Authorization': `Bearer ${this.token}` })
        };
    }

    // Authentication
    async login(username, password) {
        const response = await axios.post(`${API_BASE_URL}/login`, {
            username,
            password
        });

        if (response.data.success) {
            this.setAuthToken(response.data.token);
            this.setUser(response.data.user);
        }

        return response.data;
    }

    async logout() {
        try {
            await axios.post(`${API_BASE_URL}/logout`, {}, {
                headers: this.getHeaders()
            });
        } finally {
            this.clearAuth();
        }
    }

    // Tasks
    async getTasks() {
        const response = await axios.get(`${API_BASE_URL}/tasks`, {
            headers: this.getHeaders()
        });
        return response.data;
    }

    async getMyTasks() {
        const response = await axios.get(`${API_BASE_URL}/tasks/my`, {
            headers: this.getHeaders()
        });
        return response.data;
    }

    async getOverdueTasks() {
        const response = await axios.get(`${API_BASE_URL}/tasks/overdue`, {
            headers: this.getHeaders()
        });
        return response.data;
    }

    async createTask(title, description, deadlineDays = 7) {
        const response = await axios.post(`${API_BASE_URL}/tasks`, {
            title,
            description,
            deadlineDays
        }, {
            headers: this.getHeaders()
        });
        return response.data;
    }

    async updateTaskStatus(taskId, status) {
        const response = await axios.put(`${API_BASE_URL}/tasks/${taskId}/status`, {
            status
        }, {
            headers: this.getHeaders()
        });
        return response.data;
    }

    // Chat
    async getMessages() {
        const response = await axios.get(`${API_BASE_URL}/chat`, {
            headers: this.getHeaders()
        });
        return response.data;
    }

    async sendMessage(content) {
        const response = await axios.post(`${API_BASE_URL}/chat`, {
            content
        }, {
            headers: this.getHeaders()
        });
        return response.data;
    }

    // Users
    async getOnlineUsers() {
        const response = await axios.get(`${API_BASE_URL}/users/online`, {
            headers: this.getHeaders()
        });
        return response.data;
    }

    // Dashboard
    async getDashboard() {
        const response = await axios.get(`${API_BASE_URL}/dashboard`, {
            headers: this.getHeaders()
        });
        return response.data;
    }

    // ===== NEW API METHODS =====

    // Priority Management
    async updateTaskPriority(taskId, priority) {
        const response = await axios.put(`${API_BASE_URL}/tasks/${taskId}/priority`, {
            priority
        }, {
            headers: this.getHeaders()
        });
        return response.data;
    }

    // Task Assignment
    async assignTask(taskId, assigneeId) {
        const response = await axios.put(`${API_BASE_URL}/tasks/${taskId}/assign`, {
            assigneeId
        }, {
            headers: this.getHeaders()
        });
        return response.data;
    }

    // Task Comments
    async getTaskComments(taskId) {
        const response = await axios.get(`${API_BASE_URL}/tasks/${taskId}/comments`, {
            headers: this.getHeaders()
        });
        return response.data;
    }

    async addTaskComment(taskId, comment) {
        const response = await axios.post(`${API_BASE_URL}/tasks/${taskId}/comments`, {
            comment
        }, {
            headers: this.getHeaders()
        });
        return response.data;
    }

    // Advanced Filtering
    async getDueSoonTasks(days = 3) {
        const response = await axios.get(`${API_BASE_URL}/tasks/due-soon?days=${days}`, {
            headers: this.getHeaders()
        });
        return response.data;
    }

    async getTasksByStatus(status) {
        const response = await axios.get(`${API_BASE_URL}/tasks/by-status/${status}`, {
            headers: this.getHeaders()
        });
        return response.data;
    }

    async getTasksByProject(project) {
        const response = await axios.get(`${API_BASE_URL}/tasks/by-project/${project}`, {
            headers: this.getHeaders()
        });
        return response.data;
    }

    // Smart Assignment
    async getRecommendedAssignee() {
        const response = await axios.get(`${API_BASE_URL}/tasks/recommend-assignee`, {
            headers: this.getHeaders()
        });
        return response.data;
    }

    // Private Messaging
    async sendPrivateMessage(targetUserId, content) {
        const response = await axios.post(`${API_BASE_URL}/chat/private`, {
            targetUserId,
            content
        }, {
            headers: this.getHeaders()
        });
        return response.data;
    }

    async getPrivateMessages(userId) {
        const response = await axios.get(`${API_BASE_URL}/chat/private/${userId}`, {
            headers: this.getHeaders()
        });
        return response.data;
    }

    // Task Messages
    async getTaskMessages(taskId) {
        const response = await axios.get(`${API_BASE_URL}/tasks/${taskId}/messages`, {
            headers: this.getHeaders()
        });
        return response.data;
    }

    // Statistics
    async getStats() {
        const response = await axios.get(`${API_BASE_URL}/stats`, {
            headers: this.getHeaders()
        });
        return response.data;
    }

    // Get All Users
    async getAllUsers() {
        const response = await axios.get(`${API_BASE_URL}/users`, {
            headers: this.getHeaders()
        });
        return response.data;
    }
}

export default new APIClient();

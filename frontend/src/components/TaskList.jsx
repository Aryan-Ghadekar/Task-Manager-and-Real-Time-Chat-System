import { useState, useEffect } from 'react'
import api from '../api'
import TaskDetail from './TaskDetail'
import './TaskList.css'

function TaskList({ tasks, onTaskUpdated }) {
    const [updatingTask, setUpdatingTask] = useState(null)
    const [users, setUsers] = useState([])
    const [selectedTask, setSelectedTask] = useState(null)

    useEffect(() => {
        loadUsers()
    }, [])

    const loadUsers = async () => {
        try {
            const response = await api.getAllUsers()
            if (response.success) {
                setUsers(response.data)
            }
        } catch (error) {
            console.error('Failed to load users:', error)
        }
    }

    const handleStatusChange = async (taskId, newStatus) => {
        setUpdatingTask(taskId)
        try {
            await api.updateTaskStatus(taskId, newStatus)
            onTaskUpdated()
        } catch (error) {
            console.error('Failed to update task:', error)
        } finally {
            setUpdatingTask(null)
        }
    }

    const handlePriorityChange = async (taskId, newPriority) => {
        setUpdatingTask(taskId)
        try {
            await api.updateTaskPriority(taskId, newPriority)
            onTaskUpdated()
        } catch (error) {
            console.error('Failed to update priority:', error)
        } finally {
            setUpdatingTask(null)
        }
    }

    const handleAssignTask = async (taskId, assigneeId) => {
        setUpdatingTask(taskId)
        try {
            await api.assignTask(taskId, parseInt(assigneeId))
            onTaskUpdated()
        } catch (error) {
            console.error('Failed to assign task:', error)
        } finally {
            setUpdatingTask(null)
        }
    }

    const getStatusBadgeClass = (status) => {
        const statusMap = {
            'TODO': 'badge-todo',
            'IN_PROGRESS': 'badge-progress',
            'IN_REVIEW': 'badge-review',
            'DONE': 'badge-done',
            'BLOCKED': 'badge-blocked'
        }
        return `badge ${statusMap[status] || 'badge-todo'}`
    }

    const getPriorityColor = (priority) => {
        const colors = {
            'LOW': '#10b981',
            'MEDIUM': '#f59e0b',
            'HIGH': '#ef4444',
            'CRITICAL': '#dc2626'
        }
        return colors[priority] || '#94a3b8'
    }

    if (tasks.length === 0) {
        return (
            <div className="empty-state">
                <p>No tasks found</p>
                <span>Create a new task to get started</span>
            </div>
        )
    }

    return (
        <div className="task-list">
            {tasks.map((task) => (
                <div key={task.id} className="task-card card fade-in">
                    <div className="task-header">
                        <div className="task-title-row">
                            <h3 className="task-title">{task.title}</h3>
                            <div
                                className="priority-indicator"
                                style={{ backgroundColor: getPriorityColor(task.priority) }}
                                title={`Priority: ${task.priority}`}
                            />
                        </div>
                        <span className={getStatusBadgeClass(task.status)}>
                            {task.status.replace('_', ' ')}
                        </span>
                    </div>

                    {task.description && (
                        <p className="task-description">{task.description}</p>
                    )}

                    <div className="task-meta">
                        <div className="meta-item">
                            <span className="meta-label">Deadline:</span>
                            <span className={task.isOverdue ? 'meta-value overdue' : 'meta-value'}>
                                {task.deadline}
                                {task.isOverdue && ' (Overdue!)'}
                            </span>
                        </div>
                        <div className="meta-item">
                            <span className="meta-label">Days left:</span>
                            <span className="meta-value">{task.daysUntilDeadline}</span>
                        </div>
                    </div>

                    <div className="task-actions">
                        <div className="action-row">
                            <select
                                value={task.status}
                                onChange={(e) => handleStatusChange(task.id, e.target.value)}
                                disabled={updatingTask === task.id}
                                className="status-select"
                            >
                                <option value="TODO">To Do</option>
                                <option value="IN_PROGRESS">In Progress</option>
                                <option value="IN_REVIEW">In Review</option>
                                <option value="DONE">Done</option>
                                <option value="BLOCKED">Blocked</option>
                            </select>

                            <select
                                value={task.priority}
                                onChange={(e) => handlePriorityChange(task.id, e.target.value)}
                                disabled={updatingTask === task.id}
                                className="priority-select"
                            >
                                <option value="LOW">Low</option>
                                <option value="MEDIUM">Medium</option>
                                <option value="HIGH">High</option>
                                <option value="CRITICAL">Critical</option>
                            </select>
                        </div>

                        <div className="action-row">
                            <select
                                value={task.assigneeId || ''}
                                onChange={(e) => handleAssignTask(task.id, e.target.value)}
                                disabled={updatingTask === task.id}
                                className="assignee-select"
                            >
                                <option value="">Unassigned</option>
                                {users.map(user => (
                                    <option key={user.id} value={user.id}>
                                        {user.username} ({user.role})
                                    </option>
                                ))}
                            </select>

                            <button
                                onClick={() => setSelectedTask(task)}
                                className="btn-details"
                            >
                                View Details
                            </button>
                        </div>
                    </div>
                </div>
            ))}

            {selectedTask && (
                <TaskDetail
                    task={selectedTask}
                    onClose={() => setSelectedTask(null)}
                    onTaskUpdated={onTaskUpdated}
                />
            )}
        </div>
    )
}

export default TaskList

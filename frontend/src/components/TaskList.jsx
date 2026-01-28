import { useState } from 'react'
import api from '../api'
import './TaskList.css'

function TaskList({ tasks, onTaskUpdated }) {
    const [updatingTask, setUpdatingTask] = useState(null)

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
                    </div>
                </div>
            ))}
        </div>
    )
}

export default TaskList

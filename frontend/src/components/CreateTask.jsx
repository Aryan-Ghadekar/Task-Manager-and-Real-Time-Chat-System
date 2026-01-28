import { useState, useEffect } from 'react'
import api from '../api'
import './CreateTask.css'

function CreateTask({ onClose, onTaskCreated }) {
    const [title, setTitle] = useState('')
    const [description, setDescription] = useState('')
    const [deadlineDays, setDeadlineDays] = useState(7)
    const [priority, setPriority] = useState('MEDIUM')
    const [assigneeId, setAssigneeId] = useState('')
    const [users, setUsers] = useState([])
    const [loading, setLoading] = useState(false)
    const [loadingRecommendation, setLoadingRecommendation] = useState(false)
    const [error, setError] = useState('')

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

    const handleRecommendAssignee = async () => {
        setLoadingRecommendation(true)
        try {
            const response = await api.getRecommendedAssignee()
            if (response.success && response.data) {
                setAssigneeId(response.data.userId.toString())
            }
        } catch (error) {
            console.error('Failed to get recommendation:', error)
        } finally {
            setLoadingRecommendation(false)
        }
    }

    const handleSubmit = async (e) => {
        e.preventDefault()
        setError('')
        setLoading(true)

        try {
            const response = await api.createTask(title, description, deadlineDays)

            if (response.success && response.data) {
                const taskId = response.data.id

                // Update priority if not default
                if (priority !== 'MEDIUM') {
                    await api.updateTaskPriority(taskId, priority)
                }

                // Assign task if assignee selected
                if (assigneeId) {
                    await api.assignTask(taskId, parseInt(assigneeId))
                }

                onTaskCreated()
            } else {
                setError(response.error || 'Failed to create task')
            }
        } catch (err) {
            setError('Failed to create task. Please try again.')
        } finally {
            setLoading(false)
        }
    }

    return (
        <div className="modal-overlay" onClick={onClose}>
            <div className="modal-content" onClick={(e) => e.stopPropagation()}>
                <div className="modal-header">
                    <h2>Create New Task</h2>
                    <button className="close-btn" onClick={onClose}>×</button>
                </div>

                <form onSubmit={handleSubmit} className="create-task-form">
                    <div className="form-group">
                        <label htmlFor="title">Task Title *</label>
                        <input
                            id="title"
                            type="text"
                            value={title}
                            onChange={(e) => setTitle(e.target.value)}
                            placeholder="Enter task title"
                            required
                            autoFocus
                        />
                    </div>

                    <div className="form-group">
                        <label htmlFor="description">Description</label>
                        <textarea
                            id="description"
                            value={description}
                            onChange={(e) => setDescription(e.target.value)}
                            placeholder="Enter task description (optional)"
                            rows="4"
                        />
                    </div>

                    <div className="form-group">
                        <label htmlFor="deadline">Deadline (days from now)</label>
                        <input
                            id="deadline"
                            type="number"
                            value={deadlineDays}
                            onChange={(e) => setDeadlineDays(parseInt(e.target.value))}
                            min="1"
                            max="365"
                        />
                    </div>

                    <div className="form-group">
                        <label htmlFor="priority">Priority</label>
                        <select
                            id="priority"
                            value={priority}
                            onChange={(e) => setPriority(e.target.value)}
                            className="priority-select"
                        >
                            <option value="LOW">Low</option>
                            <option value="MEDIUM">Medium</option>
                            <option value="HIGH">High</option>
                            <option value="CRITICAL">Critical</option>
                        </select>
                    </div>

                    <div className="form-group">
                        <label htmlFor="assignee">Assignee</label>
                        <div className="assignee-controls">
                            <select
                                id="assignee"
                                value={assigneeId}
                                onChange={(e) => setAssigneeId(e.target.value)}
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
                                type="button"
                                onClick={handleRecommendAssignee}
                                className="btn-recommend"
                                disabled={loadingRecommendation}
                            >
                                {loadingRecommendation ? 'Suggesting...' : '✨ Suggest Best'}
                            </button>
                        </div>
                    </div>

                    {error && (
                        <div className="error-message">
                            {error}
                        </div>
                    )}

                    <div className="modal-actions">
                        <button type="button" className="btn btn-secondary" onClick={onClose}>
                            Cancel
                        </button>
                        <button type="submit" className="btn btn-primary" disabled={loading}>
                            {loading ? 'Creating...' : 'Create Task'}
                        </button>
                    </div>
                </form>
            </div>
        </div>
    )
}

export default CreateTask

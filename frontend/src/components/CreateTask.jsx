import { useState } from 'react'
import api from '../api'
import './CreateTask.css'

function CreateTask({ onClose, onTaskCreated }) {
    const [title, setTitle] = useState('')
    const [description, setDescription] = useState('')
    const [deadlineDays, setDeadlineDays] = useState(7)
    const [loading, setLoading] = useState(false)
    const [error, setError] = useState('')

    const handleSubmit = async (e) => {
        e.preventDefault()
        setError('')
        setLoading(true)

        try {
            const response = await api.createTask(title, description, deadlineDays)

            if (response.success) {
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

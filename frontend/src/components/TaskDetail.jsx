import { useState, useEffect } from 'react'
import api from '../api'
import './TaskDetail.css'

function TaskDetail({ task, onClose, onTaskUpdated }) {
    const [comments, setComments] = useState([])
    const [newComment, setNewComment] = useState('')
    const [loading, setLoading] = useState(true)
    const [submitting, setSubmitting] = useState(false)

    useEffect(() => {
        if (task) {
            loadComments()
        }
    }, [task])

    const loadComments = async () => {
        try {
            const response = await api.getTaskComments(task.id)
            if (response.success) {
                setComments(response.data)
            }
        } catch (error) {
            console.error('Failed to load comments:', error)
        } finally {
            setLoading(false)
        }
    }

    const handleAddComment = async (e) => {
        e.preventDefault()
        if (!newComment.trim()) return

        setSubmitting(true)
        try {
            await api.addTaskComment(task.id, newComment)
            setNewComment('')
            await loadComments()
            onTaskUpdated()
        } catch (error) {
            console.error('Failed to add comment:', error)
        } finally {
            setSubmitting(false)
        }
    }

    if (!task) return null

    return (
        <div className="modal-overlay" onClick={onClose}>
            <div className="modal-content" onClick={(e) => e.stopPropagation()}>
                <div className="modal-header">
                    <h2>Task Details</h2>
                    <button className="btn-close" onClick={onClose}>×</button>
                </div>

                <div className="modal-body">
                    <div className="task-info">
                        <h3>{task.title}</h3>
                        <p className="task-description">{task.description}</p>

                        <div className="task-details-grid">
                            <div className="detail-item">
                                <span className="detail-label">Status:</span>
                                <span className="detail-value">{task.status.replace('_', ' ')}</span>
                            </div>
                            <div className="detail-item">
                                <span className="detail-label">Priority:</span>
                                <span className="detail-value">{task.priority}</span>
                            </div>
                            <div className="detail-item">
                                <span className="detail-label">Deadline:</span>
                                <span className="detail-value">{task.deadline}</span>
                            </div>
                            <div className="detail-item">
                                <span className="detail-label">Days Left:</span>
                                <span className="detail-value">{task.daysUntilDeadline}</span>
                            </div>
                        </div>
                    </div>

                    <div className="comments-section">
                        <h4>Comments ({comments.length})</h4>

                        {loading ? (
                            <p className="loading-text">Loading comments...</p>
                        ) : comments.length === 0 ? (
                            <p className="no-comments">No comments yet. Be the first to comment!</p>
                        ) : (
                            <div className="comments-list">
                                {comments.map((comment, index) => (
                                    <div key={index} className="comment-item">
                                        <p>{comment}</p>
                                    </div>
                                ))}
                            </div>
                        )}

                        <form onSubmit={handleAddComment} className="comment-form">
                            <textarea
                                value={newComment}
                                onChange={(e) => setNewComment(e.target.value)}
                                placeholder="Add a comment..."
                                className="comment-input"
                                rows="3"
                                disabled={submitting}
                            />
                            <button
                                type="submit"
                                className="btn-submit"
                                disabled={submitting || !newComment.trim()}
                            >
                                {submitting ? 'Adding...' : 'Add Comment'}
                            </button>
                        </form>
                    </div>
                </div>
            </div>
        </div>
    )
}

export default TaskDetail

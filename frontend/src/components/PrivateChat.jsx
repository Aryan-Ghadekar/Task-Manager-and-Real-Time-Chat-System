import { useState, useEffect } from 'react'
import api from '../api'
import './PrivateChat.css'

function PrivateChat({ currentUser, onClose }) {
    const [users, setUsers] = useState([])
    const [selectedUser, setSelectedUser] = useState(null)
    const [messages, setMessages] = useState([])
    const [newMessage, setNewMessage] = useState('')
    const [loading, setLoading] = useState(false)

    useEffect(() => {
        loadUsers()
    }, [])

    useEffect(() => {
        if (selectedUser) {
            loadMessages()
            // Auto-refresh messages every 3 seconds
            const interval = setInterval(loadMessages, 3000)
            return () => clearInterval(interval)
        }
    }, [selectedUser])

    const loadUsers = async () => {
        try {
            const response = await api.getAllUsers()
            if (response.success) {
                // Filter out current user
                const otherUsers = response.data.filter(u => u.id !== currentUser.id)
                setUsers(otherUsers)
            }
        } catch (error) {
            console.error('Failed to load users:', error)
        }
    }

    const loadMessages = async () => {
        if (!selectedUser) return

        try {
            const response = await api.getPrivateMessages(selectedUser.id)
            if (response.success) {
                setMessages(response.data)
            }
        } catch (error) {
            console.error('Failed to load messages:', error)
        }
    }

    const handleSendMessage = async (e) => {
        e.preventDefault()
        if (!newMessage.trim() || !selectedUser) return

        setLoading(true)
        try {
            await api.sendPrivateMessage(selectedUser.id, newMessage)
            setNewMessage('')
            await loadMessages()
        } catch (error) {
            console.error('Failed to send message:', error)
        } finally {
            setLoading(false)
        }
    }

    return (
        <div className="modal-overlay" onClick={onClose}>
            <div className="private-chat-modal" onClick={(e) => e.stopPropagation()}>
                <div className="modal-header">
                    <h2>Private Messages</h2>
                    <button className="btn-close" onClick={onClose}>×</button>
                </div>

                <div className="private-chat-content">
                    {/* User List Sidebar */}
                    <div className="user-list-sidebar">
                        <h3>Users</h3>
                        <div className="user-list">
                            {users.map(user => (
                                <div
                                    key={user.id}
                                    className={`user-item ${selectedUser?.id === user.id ? 'active' : ''}`}
                                    onClick={() => setSelectedUser(user)}
                                >
                                    <div className="user-avatar">
                                        {user.username.charAt(0).toUpperCase()}
                                    </div>
                                    <div className="user-details">
                                        <div className="user-name">{user.username}</div>
                                        <div className="user-role">{user.role}</div>
                                    </div>
                                    {user.isOnline && <div className="online-indicator"></div>}
                                </div>
                            ))}
                        </div>
                    </div>

                    {/* Message Thread */}
                    <div className="message-thread">
                        {selectedUser ? (
                            <>
                                <div className="thread-header">
                                    <h3>Chat with {selectedUser.username}</h3>
                                </div>

                                <div className="messages-container">
                                    {messages.length === 0 ? (
                                        <div className="no-messages">
                                            <p>No messages yet. Start the conversation!</p>
                                        </div>
                                    ) : (
                                        messages.map((msg) => (
                                            <div
                                                key={msg.id}
                                                className={`message ${msg.senderId === currentUser.id ? 'sent' : 'received'}`}
                                            >
                                                <div className="message-header">
                                                    <span className="sender-name">{msg.senderName}</span>
                                                    <span className="message-time">{msg.timestamp}</span>
                                                </div>
                                                <div className="message-content">{msg.content}</div>
                                            </div>
                                        ))
                                    )}
                                </div>

                                <form onSubmit={handleSendMessage} className="message-form">
                                    <input
                                        type="text"
                                        value={newMessage}
                                        onChange={(e) => setNewMessage(e.target.value)}
                                        placeholder="Type a message..."
                                        className="message-input"
                                        disabled={loading}
                                    />
                                    <button
                                        type="submit"
                                        className="btn-send"
                                        disabled={loading || !newMessage.trim()}
                                    >
                                        {loading ? 'Sending...' : 'Send'}
                                    </button>
                                </form>
                            </>
                        ) : (
                            <div className="no-user-selected">
                                <p>Select a user to start chatting</p>
                            </div>
                        )}
                    </div>
                </div>
            </div>
        </div>
    )
}

export default PrivateChat

"""Decorator to export API implementations."""
def api_export(impl):
    def decorator(func):
        return impl
    return decorator
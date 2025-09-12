// Universal static sidebar for all pages
(function() {
    'use strict';
    
    // Check if sidebar already exists
    if (document.querySelector('.universal-sidebar')) {
        return;
    }
    
    // Create completely static sidebar HTML
    const sidebarHTML = `
        <div class="universal-sidebar" style="position: fixed !important; left: 0 !important; top: 0 !important; width: 280px !important; height: 100vh !important; z-index: 1000 !important; transform: none !important; transition: none !important;">
            <div class="sidebar-header">
                <h3>SONAR Documentation</h3>
            </div>
            <nav class="sidebar-nav">
                <ul>
                    <li><a href="index.html" data-page="index">Home</a></li>
                    <li><a href="modules.html" data-page="modules">Modules</a></li>
                    <li><a href="files.html" data-page="files">Files</a></li>
                    <li><a href="functions.html" data-page="functions">Functions</a></li>
                    <li><a href="globals.html" data-page="globals">Globals</a></li>
                    <li><a href="classes.html" data-page="classes">Classes</a></li>
                    <li><a href="namespaces.html" data-page="namespaces">Namespaces</a></li>
                </ul>
            </nav>
        </div>
    `;
    
    // Function to inject static sidebar
    function injectSidebar() {
        // Remove any existing conflicting elements
        const conflictingElements = document.querySelectorAll('#nav-tree, .ui-resizable, #side-nav');
        conflictingElements.forEach(el => {
            el.style.display = 'none';
            el.style.visibility = 'hidden';
        });
        
        // Inject sidebar at the very beginning of body
        document.body.insertAdjacentHTML('afterbegin', sidebarHTML);
        
        // Ensure sidebar stays fixed
        const sidebar = document.querySelector('.universal-sidebar');
        if (sidebar) {
            sidebar.style.position = 'fixed';
            sidebar.style.left = '0';
            sidebar.style.top = '0';
            sidebar.style.transform = 'none';
            sidebar.style.transition = 'none';
        }
        
        // Highlight current page
        const currentPage = window.location.pathname.split('/').pop().replace('.html', '') || 'index';
        const currentLink = document.querySelector(`[data-page="${currentPage}"]`);
        if (currentLink) {
            currentLink.classList.add('active');
        }
        
        // Disable any scroll or resize handlers that might affect sidebar
        window.addEventListener('scroll', function(e) {
            const sidebar = document.querySelector('.universal-sidebar');
            if (sidebar) {
                sidebar.style.position = 'fixed';
                sidebar.style.left = '0';
                sidebar.style.top = '0';
            }
        });
    }
    
    // Wait for DOM to be ready
    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', injectSidebar);
    } else {
        injectSidebar();
    }
})();
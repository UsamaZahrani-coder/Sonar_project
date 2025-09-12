/**

Doxygen Awesome
https://github.com/jothepro/doxygen-awesome-css

MIT License

Copyright (c) 2021 - 2023 jothepro

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

class DoxygenAwesomeDarkModeToggle extends HTMLElement {
    static prefersLightModeInDarkModeKey = "prefers-light-mode-in-dark-mode"
    static prefersDarkModeInLightModeKey = "prefers-dark-mode-in-light-mode"

    static _staticConstructor = function() {
        DoxygenAwesomeDarkModeToggle.darkModeEnabled = false
        DoxygenAwesomeDarkModeToggle.darkModeDefault = window.matchMedia('(prefers-color-scheme: dark)').matches
        DoxygenAwesomeDarkModeToggle.darkModeToggleEventName = "doxygen-awesome-dark-mode-toggle"
        DoxygenAwesomeDarkModeToggle.init()
        return null
    }()

    static init() {
        window.matchMedia('(prefers-color-scheme: dark)').addEventListener('change', event => {
            DoxygenAwesomeDarkModeToggle.onSystemPreferenceChanged()
        })
        document.addEventListener("DOMContentLoaded", () => {
            DoxygenAwesomeDarkModeToggle.onDOMContentLoaded()
        })
    }

    static onSystemPreferenceChanged() {
        DoxygenAwesomeDarkModeToggle.darkModeDefault = window.matchMedia('(prefers-color-scheme: dark)').matches
        DoxygenAwesomeDarkModeToggle.updateAll()
    }

    static onDOMContentLoaded() {
        if(DoxygenAwesomeDarkModeToggle.darkModeDefault) {
            const preferLightMode = (localStorage.getItem(DoxygenAwesomeDarkModeToggle.prefersLightModeInDarkModeKey) === "true")
            DoxygenAwesomeDarkModeToggle.setDarkMode(!preferLightMode)
        } else {
            const preferDarkMode = (localStorage.getItem(DoxygenAwesomeDarkModeToggle.prefersDarkModeInLightModeKey) === "true")
            DoxygenAwesomeDarkModeToggle.setDarkMode(preferDarkMode)
        }
    }

    static updateAll() {
        document.querySelectorAll("doxygen-awesome-dark-mode-toggle").forEach(toggle => {
            toggle.update()
        })
    }

    static setDarkMode(enable) {
        if(enable != DoxygenAwesomeDarkModeToggle.darkModeEnabled) {
            DoxygenAwesomeDarkModeToggle.darkModeEnabled = enable
            if(DoxygenAwesomeDarkModeToggle.darkModeEnabled) {
                document.documentElement.classList.add("dark-mode")
                document.documentElement.classList.remove("light-mode")
            } else {
                document.documentElement.classList.remove("dark-mode")
                document.documentElement.classList.add("light-mode")
            }
            localStorage.setItem(DoxygenAwesomeDarkModeToggle.prefersLightModeInDarkModeKey, (!enable).toString())
            localStorage.setItem(DoxygenAwesomeDarkModeToggle.prefersDarkModeInLightModeKey, enable.toString())
            DoxygenAwesomeDarkModeToggle.notify()
        }
    }

    static notify() {
        const event = new CustomEvent(DoxygenAwesomeDarkModeToggle.darkModeToggleEventName, {
            detail: {
                darkMode: DoxygenAwesomeDarkModeToggle.darkModeEnabled
            }
        })
        document.dispatchEvent(event)
    }

    constructor() {
        super();
        this.onclick=this.toggleDarkMode
    }

    update() {
        this.innerHTML = `<div class="dark-mode-toggle">
            <span class="dark-mode-toggle-icon" title="Toggle Light/Dark Mode">
                <svg class="light-mode" width="24" height="24" viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
                    <path d="M12 16C14.2091 16 16 14.2091 16 12C16 9.79086 14.2091 8 12 8C9.79086 8 8 9.79086 8 12C8 14.2091 9.79086 16 12 16Z" fill="currentColor"/>
                    <path d="M12 2C12.2652 2 12.5196 2.10536 12.7071 2.29289C12.8946 2.48043 13 2.73478 13 3V4C13 4.26522 12.8946 4.51957 12.7071 4.70711C12.5196 4.89464 12.2652 5 12 5C11.7348 5 11.4804 4.89464 11.2929 4.70711C11.1054 4.51957 11 4.26522 11 4V3C11 2.73478 11.1054 2.48043 11.2929 2.29289C11.4804 2.10536 11.7348 2 12 2V2ZM19.071 4.92893C19.2585 5.11647 19.3639 5.37082 19.3639 5.63604C19.3639 5.90126 19.2585 6.15561 19.071 6.34314L18.364 7.05014C18.1765 7.23767 17.9221 7.34303 17.6569 7.34303C17.3917 7.34303 17.1373 7.23767 16.9498 7.05014C16.7623 6.8626 16.6569 6.60825 16.6569 6.34303C16.6569 6.07782 16.7623 5.82346 16.9498 5.63593L17.6568 4.92893C17.8444 4.74139 18.0987 4.63604 18.364 4.63604C18.6292 4.63604 18.8835 4.74139 19.071 4.92893V4.92893ZM22 12C22 12.2652 21.8946 12.5196 21.7071 12.7071C21.5196 12.8946 21.2652 13 21 13H20C19.7348 13 19.4804 12.8946 19.2929 12.7071C19.1054 12.5196 19 12.2652 19 12C19 11.7348 19.1054 11.4804 19.2929 11.2929C19.4804 11.1054 19.7348 11 20 11H21C21.2652 11 21.5196 11.1054 21.7071 11.2929C21.8946 11.4804 22 11.7348 22 12ZM19.071 17.657C19.2585 17.8445 19.3639 18.0989 19.3639 18.3641C19.3639 18.6293 19.2585 18.8837 19.071 19.0712L18.364 19.778C18.1765 19.9655 17.9221 20.0709 17.6569 20.0709C17.3917 20.0709 17.1373 19.9655 16.9498 19.778C16.7623 19.5905 16.6569 19.3361 16.6569 19.0709C16.6569 18.8057 16.7623 18.5513 16.9498 18.3638L17.6568 17.657C17.8444 17.4694 18.0987 17.3641 18.364 17.3641C18.6292 17.3641 18.8835 17.4694 19.071 17.657V17.657ZM12 19C12.2652 19 12.5196 19.1054 12.7071 19.2929C12.8946 19.4804 13 19.7348 13 20V21C13 21.2652 12.8946 21.5196 12.7071 21.7071C12.5196 21.8946 12.2652 22 12 22C11.7348 22 11.4804 21.8946 11.2929 21.7071C11.1054 21.5196 11 21.2652 11 21V20C11 19.7348 11.1054 19.4804 11.2929 19.2929C11.4804 19.1054 11.7348 19 12 19V19ZM4.929 17.657C5.11654 17.4694 5.37089 17.3641 5.63611 17.3641C5.90133 17.3641 6.15568 17.4694 6.34321 17.657L7.05021 18.364C7.23775 18.5515 7.3431 18.8059 7.3431 19.0711C7.3431 19.3363 7.23775 19.5907 7.05021 19.7782C6.86268 19.9657 6.60832 20.0711 6.34311 20.0711C6.07789 20.0711 5.82354 19.9657 5.636 19.7782L4.929 19.0712C4.74146 18.8837 4.63611 18.6293 4.63611 18.3641C4.63611 18.0989 4.74146 17.8445 4.929 17.657V17.657ZM2 12C2 11.7348 2.10536 11.4804 2.29289 11.2929C2.48043 11.1054 2.73478 11 3 11H4C4.26522 11 4.51957 11.1054 4.70711 11.2929C4.89464 11.4804 5 11.7348 5 12C5 12.2652 4.89464 12.5196 4.70711 12.7071C4.51957 12.8946 4.26522 13 4 13H3C2.73478 13 2.48043 12.8946 2.29289 12.7071C2.10536 12.5196 2 12.2652 2 12V12ZM4.929 4.92893C5.11654 4.74139 5.37089 4.63604 5.63611 4.63604C5.90133 4.63604 6.15568 4.74139 6.34321 4.92893L7.05021 5.63593C7.23775 5.82346 7.3431 6.07782 7.3431 6.34303C7.3431 6.60825 7.23775 6.8626 7.05021 7.05014C6.86268 7.23767 6.60832 7.34303 6.34311 7.34303C6.07789 7.34303 5.82354 7.23767 5.636 7.05014L4.929 6.34314C4.74146 6.15561 4.63611 5.90126 4.63611 5.63604C4.63611 5.37082 4.74146 5.11647 4.929 4.92893V4.92893Z" fill="currentColor"/>
                </svg>
                <svg class="dark-mode" width="24" height="24" viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
                    <path d="M12.0557 3.59974C12.2752 3.2813 12.2913 2.86484 12.0972 2.53033C11.9031 2.19582 11.5335 2.00324 11.1492 2.03579C6.35488 2.46868 2.64612 6.45342 2.6416 11.3306C2.63903 14.1457 3.71315 16.8503 5.64547 18.8592C7.57778 20.8681 10.2055 21.9999 12.9557 22C17.7641 22 21.7166 18.1275 22.0557 13.3725C22.0882 12.9882 21.8957 12.6186 21.5612 12.4245C21.2267 12.2304 20.8102 12.2465 20.4918 12.466C19.5833 13.1087 18.5127 13.4586 17.4005 13.4767C16.2884 13.4948 15.2089 13.1803 14.2814 12.5679C13.3539 11.9555 12.6159 11.0726 12.1593 10.0288C11.7028 8.98496 11.5474 7.82763 11.7121 6.69709C11.8768 5.56654 12.3547 4.50934 13.0918 3.64574C13.2854 3.41313 13.3353 3.0961 13.2239 2.81362C13.1124 2.53114 12.8543 2.31887 12.5557 2.25574C11.7601 2.10035 10.9447 2.0378 10.1416 2.0725C10.8461 2.44194 11.4891 2.9481 12.0557 3.59974Z" fill="currentColor"/>
                </svg>
            </span>
        </div>`
    }

    toggleDarkMode() {
        DoxygenAwesomeDarkModeToggle.setDarkMode(!DoxygenAwesomeDarkModeToggle.darkModeEnabled)
    }
}

customElements.define("doxygen-awesome-dark-mode-toggle", DoxygenAwesomeDarkModeToggle)
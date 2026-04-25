/* ── Kraken2D Landing — main.js ── */

// ── Custom cursor ──────────────────────────────────────────────
const cursor = document.getElementById('cursor');

document.addEventListener('mousemove', e => {
  cursor.style.left = e.clientX + 'px';
  cursor.style.top  = e.clientY + 'px';
});

// Expand cursor on interactive elements
const hoverTargets = document.querySelectorAll(
  'a, button, .game-card, .feature-card, .feature, .control-card'
);
hoverTargets.forEach(el => {
  el.addEventListener('mouseenter', () => cursor.classList.add('expanded'));
  el.addEventListener('mouseleave', () => cursor.classList.remove('expanded'));
});

// ── Floating pixel particles in hero ──────────────────────────
const pixelContainer = document.getElementById('pixels');
const COLORS = ['#00f5ff', '#39ff14', '#ff2d78', '#ffe600', '#9d4edd'];

function spawnPixel() {
  const p = document.createElement('div');
  p.className = 'pixel';
  const size   = 2 + Math.random() * 5;
  const color  = COLORS[Math.floor(Math.random() * COLORS.length)];
  const left   = Math.random() * 100;
  const dur    = 10 + Math.random() * 18;
  const delay  = -Math.random() * dur;

  p.style.cssText = `
    width: ${size}px;
    height: ${size}px;
    left: ${left}%;
    background: ${color};
    box-shadow: 0 0 ${size * 2}px ${color};
    animation-duration: ${dur}s;
    animation-delay: ${delay}s;
    opacity: ${0.3 + Math.random() * 0.5};
  `;
  pixelContainer.appendChild(p);
}

for (let i = 0; i < 35; i++) spawnPixel();

// ── Mobile burger menu ─────────────────────────────────────────
const burger     = document.getElementById('burger');
const mobileMenu = document.getElementById('mobileMenu');

burger.addEventListener('click', () => {
  mobileMenu.classList.toggle('open');
  burger.textContent = mobileMenu.classList.contains('open') ? '✕' : '☰';
});

function closeMobile() {
  mobileMenu.classList.remove('open');
  burger.textContent = '☰';
}

// Close on outside click
document.addEventListener('click', e => {
  if (!burger.contains(e.target) && !mobileMenu.contains(e.target)) {
    closeMobile();
  }
});

// ── Nav background on scroll ───────────────────────────────────
const nav = document.getElementById('nav');

window.addEventListener('scroll', () => {
  if (window.scrollY > 60) {
    nav.style.background = 'rgba(4,5,13,0.97)';
    nav.style.borderBottomColor = 'rgba(0,245,255,0.2)';
  } else {
    nav.style.background = 'rgba(4,5,13,0.88)';
    nav.style.borderBottomColor = 'rgba(0,245,255,0.12)';
  }
}, { passive: true });

// ── Scroll reveal ──────────────────────────────────────────────
const revealEls = document.querySelectorAll(
  '.game-card, .feature, .feature-card, .control-card, ' +
  '.stat, .arch-block, .code-block, .req-item'
);

revealEls.forEach(el => el.classList.add('reveal'));

const revealObserver = new IntersectionObserver(entries => {
  entries.forEach((entry, i) => {
    if (entry.isIntersecting) {
      // Stagger sibling cards
      const siblings = [...entry.target.parentElement.children];
      const idx = siblings.indexOf(entry.target);
      entry.target.style.transitionDelay = `${idx * 60}ms`;
      entry.target.classList.add('visible');
      revealObserver.unobserve(entry.target);
    }
  });
}, { threshold: 0.1 });

revealEls.forEach(el => revealObserver.observe(el));

// ── Animated stat counters ─────────────────────────────────────
function animateCounter(el, target, suffix = '') {
  // Only animate purely numeric targets
  if (isNaN(parseFloat(target))) return;

  const num     = parseFloat(target);
  const dur     = 1200;
  const start   = performance.now();

  function tick(now) {
    const elapsed = now - start;
    const progress = Math.min(elapsed / dur, 1);
    // Ease out cubic
    const eased = 1 - Math.pow(1 - progress, 3);
    const current = Math.round(eased * num);
    el.textContent = current + suffix;
    if (progress < 1) requestAnimationFrame(tick);
    else el.textContent = target + suffix;
  }
  requestAnimationFrame(tick);
}

const statNums = document.querySelectorAll('.stat-num');
const statObserver = new IntersectionObserver(entries => {
  entries.forEach(entry => {
    if (entry.isIntersecting) {
      const el      = entry.target;
      const original = el.textContent;
      animateCounter(el, original);
      statObserver.unobserve(el);
    }
  });
}, { threshold: 0.8 });

statNums.forEach(s => statObserver.observe(s));

// ── Glitch effect on hero title ────────────────────────────────
const heroTitle = document.querySelector('.hero-title');

function glitch() {
  heroTitle.style.textShadow = `
    ${Math.random() * 6 - 3}px 0 var(--pink),
    ${Math.random() * 6 - 3}px 0 var(--cyan)
  `;
  setTimeout(() => {
    heroTitle.style.textShadow = '';
    // Schedule next glitch randomly between 4–12 seconds
    setTimeout(glitch, 4000 + Math.random() * 8000);
  }, 80 + Math.random() * 80);
}

setTimeout(glitch, 3000);

// ── Active nav link highlight ──────────────────────────────────
const sections = document.querySelectorAll('section[id], div[id="hero"]');
const navLinks  = document.querySelectorAll('.nav-links a');

const sectionObserver = new IntersectionObserver(entries => {
  entries.forEach(entry => {
    if (entry.isIntersecting) {
      navLinks.forEach(link => {
        link.style.color = '';
        link.style.textShadow = '';
        if (link.getAttribute('href') === '#' + entry.target.id) {
          link.style.color = 'var(--cyan)';
          link.style.textShadow = '0 0 8px var(--cyan)';
        }
      });
    }
  });
}, { threshold: 0.4 });

sections.forEach(s => sectionObserver.observe(s));

R"rawliteral(
.hero-header {
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 16px;
  margin-bottom: 18px;
}
.app-illustration svg {
  width: 65px;
  height: auto;
  display: block;
  transform: translateY(-6px);
  filter: drop-shadow(0 16px 28px rgba(17, 24, 39, 0.2));
}
.hero-bubble {
  animation: hero-bubble-rise 2.4s ease-in-out infinite;
  transform-box: fill-box;
}
.hero-bubble-1 { animation-delay: 0s; }
.hero-bubble-2 { animation-delay: 0.4s; }
.hero-bubble-3 { animation-delay: 0.8s; }
@keyframes hero-bubble-rise {
  0% { transform: translateY(0px); opacity: 0.9; }
  50% { transform: translateY(-6px); opacity: 0.55; }
  100% { transform: translateY(0px); opacity: 0.9; }
}
.app-illustration {
  display: inline-flex;
  align-items: center;
  justify-content: center;
}
.app-illustration.is-escaping {
  animation: hero-fish-escape-left 2.1s cubic-bezier(0.22, 0.7, 0.2, 1) 1;
}
.hero-svg-fish {
  animation: hero-fish-swim 4.2s ease-in-out infinite;
  transform-origin: 32px 42px;
  transform-box: fill-box;
  cursor: pointer;
}
.hero-fish-body {
  animation: hero-fish-body-wave 2.2s ease-in-out infinite;
  transform-origin: 29px 40px;
  transform-box: fill-box;
}
.hero-fish-tail {
  animation: hero-fish-tail-flick 1.25s ease-in-out infinite;
  transform-origin: 38px 56px;
  transform-box: fill-box;
}
.hero-fish-tail.tail-burst {
  animation: hero-fish-tail-burst 0.16s ease-in-out 6;
}
.hero-fish-eye {
  transform-origin: center;
  transform-box: fill-box;
  animation: hero-fish-eye-blink 6s linear infinite;
}
@keyframes hero-fish-swim {
  0%, 100% { transform: translate3d(0px, 0px, 0) rotate(-0.6deg); }
  25% { transform: translate3d(1.8px, -2.4px, 0) rotate(0.5deg); }
  50% { transform: translate3d(3.2px, -0.8px, 0) rotate(0.9deg); }
  75% { transform: translate3d(1px, -3px, 0) rotate(0.1deg); }
}
@keyframes hero-fish-escape-left {
  0% { transform: translate3d(0px, 0px, 0); }
  22% { transform: translate3d(-16px, -1px, 0); }
  48% { transform: translate3d(-42px, -3px, 0); }
  72% { transform: translate3d(-10px, 0px, 0); }
  86% { transform: translate3d(6px, 0px, 0); }
  100% { transform: translate3d(0px, 0px, 0); }
}
@keyframes hero-fish-body-wave {
  0%, 100% { transform: rotate(0deg); }
  50% { transform: rotate(1.6deg); }
}
@keyframes hero-fish-tail-flick {
  0%, 100% { transform: rotate(0deg); }
  35% { transform: rotate(-5deg); }
  65% { transform: rotate(4deg); }
}
@keyframes hero-fish-tail-burst {
  0% { transform: rotate(0deg); }
  50% { transform: rotate(-16deg); }
  100% { transform: rotate(14deg); }
}
@keyframes hero-fish-eye-blink {
  0%, 47%, 49%, 100% { transform: scaleY(1); }
  48% { transform: scaleY(0.15); }
}
.app-heading {
  display: flex;
  flex-direction: column;
  align-items: flex-start;
  gap: 2px;
}
.app-title {
  font-size: 24px;
  font-weight: 700;
  color: #1f2937;
  letter-spacing: 0.4px;
}
.app-subtitle {
  font-size: 13px;
  color: #6b7280;
  letter-spacing: 0.3px;
}
)rawliteral"

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
.hero-svg-fish {
  animation: hero-fish-bob 4.5s ease-in-out infinite;
  transform-origin: 32px 42px;
  transform-box: fill-box;
}
@keyframes hero-fish-bob {
  0%, 100% { transform: translateY(0px); }
  50% { transform: translateY(-3.5px); }
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

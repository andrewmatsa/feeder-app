.bottom-tabs {
  position: fixed;
  bottom: 0;
  left: 0;
  right: 0;
  display: flex;
  background: rgba(255,255,255,0.96);
  box-shadow: 0 -6px 18px rgba(0, 0, 0, 0.15);
  z-index: 1000;
  border-top: 1px solid rgba(15, 23, 42, 0.08);
  padding: 10px 12px;
  border-radius: 16px 16px 0 0;
  backdrop-filter: blur(10px);
}
.bottom-tab {
  flex: 1;
  padding: 10px 8px;
  text-align: center;
  text-decoration: none;
  color: #4b5563;
  font-size: 12px;
  font-weight: 500;
  transition: all 0.2s ease;
  border: none;
  background: transparent;
  cursor: pointer;
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 4px;
  border-radius: 12px;
  margin: 0 6px;
}
.bottom-tab:hover {
  background: rgba(15, 23, 42, 0.06);
}
.bottom-tab.active {
  color: #111827;
  background: rgba(15, 23, 42, 0.1);
}
.bottom-tab-icon {
  font-size: 22px;
  line-height: 1;
  color: inherit;
  display: inline-flex;
}
.bottom-tab-icon-svg {
  width: 22px;
  height: 22px;
  stroke: currentColor;
  fill: none;
  stroke-width: 1.9;
  stroke-linecap: round;
  stroke-linejoin: round;
}
.bottom-tab-icon-svg .filled {
  fill: currentColor;
  stroke: none;
}
.bottom-tab.active .bottom-tab-icon,
.bottom-tab.active .home-icon {
  color: #111827;
}

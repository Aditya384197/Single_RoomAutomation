#pragma once
#include <Arduino.h>

const char DASHBOARD_HTML[] PROGMEM = R"HTMLPAGE(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <title>Smart Automation</title>
    <style>
        :root {
            --bg-body: #0d1117;
            --bg-card: #161b22;
            --bg-card-hover: #1c2333;
            --border-subtle: #30363d;
            --green-bright: #2ea043;
            --green-dim: #238636;
            --text-primary: #f0f6fc;
            --text-secondary: #8b949e;
            --text-muted: #484f58;
            --radius-card: 16px;
            --radius-sm: 8px;
            --transition-butter: all 0.35s cubic-bezier(0.34, 1.56, 0.64, 1);
            --transition-smooth: all 0.4s cubic-bezier(0.25, 0.46, 0.45, 0.94);
            --font: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
        }

        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
            -webkit-tap-highlight-color: transparent;
        }

        body {
            background: var(--bg-body);
            color: var(--text-primary);
            font-family: var(--font);
            padding: 16px 10px 30px;
            display: flex;
            justify-content: center;
            min-height: 100vh;
            -webkit-font-smoothing: antialiased;
            -moz-osx-font-smoothing: grayscale;
        }

        .app {
            max-width: 480px;
            width: 100%;
        }

        .header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 8px 4px 20px;
            border-bottom: 1px solid var(--border-subtle);
            margin-bottom: 24px;
            transition: var(--transition-smooth);
        }

        .header-left {
            display: flex;
            align-items: center;
            gap: 10px;
        }

        .header-left h1 {
            font-size: 18px;
            font-weight: 700;
            letter-spacing: -0.3px;
            color: var(--text-primary);
        }

        .btn-settings {
            background: var(--bg-card);
            border: 1px solid var(--border-subtle);
            border-radius: 40px;
            padding: 6px 10px;
            color: var(--text-secondary);
            font-size: 18px;
            cursor: pointer;
            transition: var(--transition-butter);
            display: flex;
            align-items: center;
            justify-content: center;
            line-height: 1;
        }
        .btn-settings:hover {
            border-color: var(--green-dim);
            color: var(--text-primary);
            transform: rotate(60deg);
        }
        .btn-settings:active {
            transform: scale(0.90) rotate(60deg);
        }

        .header-right {
            display: flex;
            align-items: center;
        }

        .clock {
            font-size: 15px;
            font-weight: 600;
            font-variant-numeric: tabular-nums;
            color: var(--text-primary);
            letter-spacing: 0.3px;
            background: var(--bg-card);
            padding: 6px 14px;
            border-radius: 40px;
            border: 1px solid var(--border-subtle);
            transition: var(--transition-butter);
        }

        .settings-overlay {
            position: fixed;
            inset: 0;
            background: rgba(0, 0, 0, 0.6);
            backdrop-filter: blur(8px);
            -webkit-backdrop-filter: blur(8px);
            z-index: 999;
            opacity: 0;
            pointer-events: none;
            transition: opacity 0.4s ease;
        }
        .settings-overlay.open {
            opacity: 1;
            pointer-events: all;
        }

        .settings-panel {
            position: fixed;
            top: 0;
            right: -100%;
            width: 85%;
            max-width: 380px;
            height: 100%;
            background: var(--bg-body);
            border-left: 1px solid var(--border-subtle);
            z-index: 1000;
            padding: 24px 0 30px;
            overflow-y: auto;
            transition: right 0.5s cubic-bezier(0.34, 1.56, 0.64, 1);
            box-shadow: -10px 0 50px rgba(0, 0, 0, 0.5);
            display: flex;
            flex-direction: column;
        }
        .settings-panel.open {
            right: 0;
        }

        .settings-panel .panel-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 0 20px 16px;
            border-bottom: 1px solid var(--border-subtle);
            margin-bottom: 16px;
            flex-shrink: 0;
            min-height: 56px;
        }
        .settings-panel .panel-header .title-area {
            display: flex;
            align-items: center;
            gap: 8px;
        }
        .settings-panel .panel-header .back-btn {
            background: none;
            border: none;
            color: var(--text-secondary);
            font-size: 22px;
            cursor: pointer;
            padding: 0 4px;
            transition: var(--transition-butter);
            display: none;
            line-height: 1;
        }
        .settings-panel .panel-header .back-btn:active {
            transform: scale(0.85);
        }
        .settings-panel .panel-header .back-btn.show {
            display: block;
        }

        .settings-panel .panel-header h2 {
            font-size: 18px;
            font-weight: 700;
            color: var(--text-primary);
            transition: var(--transition-smooth);
        }
        .settings-panel .panel-header .close-btn {
            background: none;
            border: none;
            color: var(--text-secondary);
            font-size: 24px;
            cursor: pointer;
            padding: 4px 8px;
            transition: var(--transition-butter);
            flex-shrink: 0;
        }
        .settings-panel .panel-header .close-btn:active {
            transform: scale(0.85);
        }

        .settings-content {
            flex: 1;
            padding: 0 20px 20px;
            position: relative;
            overflow: hidden;
        }

        .settings-page {
            transition: var(--transition-smooth);
            opacity: 1;
            transform: translateX(0);
            position: relative;
        }
        .settings-page.slide-out {
            opacity: 0;
            transform: translateX(-20px);
            position: absolute;
            pointer-events: none;
            width: calc(100% - 40px);
        }
        .settings-page.slide-in {
            opacity: 0;
            transform: translateX(20px);
            position: absolute;
            pointer-events: none;
            width: calc(100% - 40px);
        }
        .settings-page.slide-in.active {
            opacity: 1;
            transform: translateX(0);
            position: relative;
            pointer-events: all;
        }

        .menu-list {
            display: flex;
            flex-direction: column;
            gap: 8px;
        }
        .menu-item {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 16px 18px;
            background: var(--bg-card);
            border: 1px solid var(--border-subtle);
            border-radius: var(--radius-sm);
            cursor: pointer;
            transition: var(--transition-butter);
            font-size: 15px;
            color: var(--text-primary);
        }
        .menu-item:active {
            transform: scale(0.97);
            background: var(--bg-card-hover);
        }
        .menu-item .arrow {
            color: var(--text-muted);
            font-size: 18px;
        }
        .menu-item .desc {
            font-size: 11px;
            color: var(--text-secondary);
            font-weight: 400;
            margin-top: 2px;
        }

        .sub-group {
            margin-bottom: 16px;
        }
        .sub-group label {
            font-size: 11px;
            text-transform: uppercase;
            letter-spacing: 1px;
            color: var(--text-muted);
            font-weight: 700;
            display: block;
            margin-bottom: 6px;
        }
        .sub-group .inp {
            width: 100%;
            padding: 10px 14px;
            background: var(--bg-card);
            border: 1px solid var(--border-subtle);
            border-radius: var(--radius-sm);
            color: var(--text-primary);
            font-size: 14px;
            outline: none;
            transition: var(--transition-smooth);
            font-family: var(--font);
        }
        .sub-group .inp:focus {
            border-color: var(--green-dim);
            box-shadow: 0 0 15px rgba(46, 160, 67, 0.03);
        }
        .sub-group .file-inp {
            padding: 8px 10px;
            background: var(--bg-card);
            border: 1px solid var(--border-subtle);
            border-radius: var(--radius-sm);
            color: var(--text-secondary);
            font-size: 12px;
            width: 100%;
            font-family: var(--font);
        }
        .btn-row {
            display: flex;
            gap: 8px;
            margin-top: 4px;
        }
        .btn-row .btn {
            flex: 1;
            padding: 10px;
            border: 1px solid var(--border-subtle);
            border-radius: var(--radius-sm);
            background: var(--bg-card);
            color: var(--text-secondary);
            font-size: 12px;
            font-weight: 700;
            text-transform: uppercase;
            letter-spacing: 0.5px;
            cursor: pointer;
            transition: var(--transition-butter);
            text-align: center;
            font-family: var(--font);
        }
        .btn-row .btn:active {
            transform: scale(0.95);
        }
        .btn-row .btn-primary {
            background: var(--green-dim);
            color: #fff;
            border-color: var(--green-dim);
            box-shadow: 0 2px 10px rgba(46, 160, 67, 0.15);
        }
        .btn-row .btn-danger {
            border-color: rgba(255, 0, 0, 0.15);
            color: #f85149;
        }
        .btn-back-bottom {
            width: 100%;
            padding: 12px;
            border: 1px solid var(--border-subtle);
            border-radius: var(--radius-sm);
            background: transparent;
            color: var(--text-secondary);
            font-size: 13px;
            font-weight: 600;
            cursor: pointer;
            transition: var(--transition-butter);
            text-align: center;
            font-family: var(--font);
            margin-top: 20px;
            letter-spacing: 0.5px;
        }
        .btn-back-bottom:active {
            transform: scale(0.96);
            background: rgba(255, 255, 255, 0.02);
        }

        .wifi-list {
            display: flex;
            flex-direction: column;
            gap: 6px;
            margin-top: 6px;
        }
        .wifi-item {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 10px 14px;
            background: var(--bg-card);
            border: 1px solid var(--border-subtle);
            border-radius: var(--radius-sm);
            cursor: pointer;
            transition: var(--transition-butter);
            font-size: 14px;
            color: var(--text-primary);
        }
        .wifi-item:active {
            transform: scale(0.97);
            background: var(--bg-card-hover);
        }
        .wifi-item .signal {
            color: var(--text-muted);
            font-size: 12px;
        }
        .wifi-item.hidden-item {
            border-style: dashed;
            border-color: var(--text-muted);
            color: var(--text-secondary);
        }
        .wifi-status-text {
            font-size: 11px;
            color: var(--text-muted);
            margin-top: 6px;
            font-style: italic;
        }

        .card {
            background: var(--bg-card);
            border: 1px solid var(--border-subtle);
            border-radius: var(--radius-card);
            padding: 20px 18px 18px;
            margin-bottom: 24px;
            transition: var(--transition-butter);
        }
        .card.active {
            border-color: var(--green-dim);
            background: var(--bg-card-hover);
            box-shadow: 0 0 25px rgba(46, 160, 67, 0.03), inset 0 0 15px rgba(46, 160, 67, 0.02);
        }

        .card-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 14px;
        }

        .device-title {
            font-size: 16px;
            font-weight: 600;
            display: flex;
            align-items: center;
            gap: 10px;
        }
        .device-title .icon {
            width: 22px;
            height: 22px;
            stroke: var(--text-secondary);
            stroke-width: 2;
            fill: none;
            transition: var(--transition-butter);
        }
        .card.active .device-title .icon {
            stroke: var(--green-bright);
            filter: drop-shadow(0 0 6px rgba(46, 160, 67, 0.2));
        }

        .badge {
            font-size: 10px;
            font-weight: 600;
            text-transform: uppercase;
            letter-spacing: 0.5px;
            color: var(--text-secondary);
            background: rgba(255, 255, 255, 0.04);
            padding: 4px 12px;
            border-radius: 30px;
            border: 1px solid var(--border-subtle);
            transition: var(--transition-smooth);
        }
        .card.active .badge {
            color: var(--green-bright);
            border-color: rgba(46, 160, 67, 0.15);
        }

        .toggle-row {
            display: flex;
            justify-content: space-between;
            align-items: center;
            background: var(--bg-body);
            border-radius: var(--radius-sm);
            padding: 8px 14px 8px 16px;
            margin-bottom: 14px;
            border: 1px solid var(--border-subtle);
            transition: var(--transition-smooth);
        }
        .toggle-label {
            font-size: 14px;
            font-weight: 500;
            color: var(--text-secondary);
        }
        .toggle {
            position: relative;
            width: 48px;
            height: 26px;
            flex-shrink: 0;
            cursor: pointer;
        }
        .toggle input {
            opacity: 0;
            width: 0;
            height: 0;
        }
        .toggle .slider {
            position: absolute;
            inset: 0;
            background: var(--text-muted);
            border-radius: 30px;
            transition: var(--transition-butter);
            box-shadow: inset 0 2px 4px rgba(0, 0, 0, 0.3);
        }
        .toggle .slider::before {
            content: "";
            position: absolute;
            height: 18px;
            width: 18px;
            left: 4px;
            bottom: 4px;
            background: #ffffff;
            border-radius: 50%;
            transition: var(--transition-butter);
            box-shadow: 0 2px 6px rgba(0, 0, 0, 0.3);
        }
        .toggle input:checked+.slider {
            background: var(--green-dim);
            box-shadow: 0 0 20px rgba(46, 160, 67, 0.2), inset 0 2px 4px rgba(0, 0, 0, 0.2);
        }
        .toggle input:checked+.slider::before {
            transform: translateX(22px);
            background: #ffffff;
            box-shadow: 0 2px 12px rgba(46, 160, 67, 0.4);
        }

        .schedule-section {
            border-top: 1px solid var(--border-subtle);
            padding-top: 14px;
            margin-top: 2px;
            margin-bottom: 16px;
        }
        .schedule-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 10px;
        }
        .schedule-header .section-label {
            font-size: 11px;
            font-weight: 700;
            text-transform: uppercase;
            letter-spacing: 0.8px;
            color: var(--text-secondary);
        }
        .toggle-mini {
            position: relative;
            width: 36px;
            height: 20px;
            flex-shrink: 0;
            cursor: pointer;
        }
        .toggle-mini input {
            opacity: 0;
            width: 0;
            height: 0;
        }
        .toggle-mini .slider-mini {
            position: absolute;
            inset: 0;
            background: var(--text-muted);
            border-radius: 30px;
            transition: var(--transition-butter);
        }
        .toggle-mini .slider-mini::before {
            content: "";
            position: absolute;
            height: 14px;
            width: 14px;
            left: 3px;
            bottom: 3px;
            background: #ffffff;
            border-radius: 50%;
            transition: var(--transition-butter);
            box-shadow: 0 1px 3px rgba(0, 0, 0, 0.3);
        }
        .toggle-mini input:checked+.slider-mini {
            background: var(--green-dim);
            box-shadow: 0 0 15px rgba(46, 160, 67, 0.15);
        }
        .toggle-mini input:checked+.slider-mini::before {
            transform: translateX(16px);
        }

        .schedule-row {
            display: flex;
            flex-wrap: wrap;
            align-items: center;
            gap: 6px;
        }
        .time-group {
            display: flex;
            align-items: center;
            gap: 2px;
            background: var(--bg-body);
            padding: 4px 8px;
            border-radius: var(--radius-sm);
            border: 1px solid var(--border-subtle);
            transition: var(--transition-smooth);
        }
        .time-group:focus-within {
            border-color: var(--green-dim);
            box-shadow: 0 0 15px rgba(46, 160, 67, 0.03);
        }
        .time-group .time-label {
            font-size: 9px;
            font-weight: 700;
            color: var(--text-secondary);
            text-transform: uppercase;
            letter-spacing: 0.5px;
            margin-right: 2px;
        }
        .time-group input {
            width: 32px;
            padding: 5px 0;
            background: transparent;
            border: none;
            color: var(--text-primary);
            font-size: 13px;
            font-weight: 600;
            text-align: center;
            outline: none;
            font-family: var(--font);
        }
        .time-group input:focus {
            background: rgba(255, 255, 255, 0.03);
            border-radius: 4px;
        }
        .time-group .colon {
            color: var(--text-muted);
            font-weight: 700;
            font-size: 14px;
            padding: 0 1px;
        }
        .time-group select {
            background: transparent;
            border: none;
            color: var(--text-secondary);
            font-size: 11px;
            font-weight: 700;
            padding: 4px 0;
            outline: none;
            cursor: pointer;
            font-family: var(--font);
            border-left: 1px solid var(--border-subtle);
            padding-left: 4px;
            margin-left: 2px;
            transition: var(--transition-smooth);
        }
        .time-group select:focus {
            color: var(--text-primary);
        }

        .btn-save-schedule {
            padding: 8px 14px;
            background: var(--green-dim);
            color: #fff;
            border: none;
            border-radius: var(--radius-sm);
            font-size: 10px;
            font-weight: 700;
            text-transform: uppercase;
            letter-spacing: 0.5px;
            cursor: pointer;
            transition: var(--transition-butter);
            font-family: var(--font);
            white-space: nowrap;
            box-shadow: 0 2px 8px rgba(46, 160, 67, 0.1);
        }
        .btn-save-schedule:active {
            transform: scale(0.94);
        }

        .schedule-feedback {
            font-size: 11px;
            color: var(--text-secondary);
            margin-top: 6px;
            font-weight: 500;
            transition: var(--transition-smooth);
            opacity: 0.9;
        }
        .schedule-feedback.ok {
            color: var(--green-bright);
            opacity: 1;
        }

        .timer-grid {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 12px;
        }
        .pod {
            background: var(--bg-body);
            border-radius: var(--radius-sm);
            padding: 14px 10px 12px;
            border: 1px solid var(--border-subtle);
            transition: var(--transition-butter);
            text-align: center;
        }
        .pod.active {
            border-color: var(--green-dim);
            background: rgba(46, 160, 67, 0.04);
            box-shadow: 0 0 20px rgba(46, 160, 67, 0.02);
        }
        .pod-title {
            font-size: 9px;
            text-transform: uppercase;
            letter-spacing: 1.2px;
            color: var(--text-secondary);
            font-weight: 700;
            margin-bottom: 6px;
            transition: var(--transition-smooth);
        }
        .pod.active .pod-title {
            color: var(--green-bright);
        }

        .circle-wrap {
            position: relative;
            width: 68px;
            height: 68px;
            margin: 0 auto 6px;
        }
        .circle-wrap svg {
            transform: rotate(-90deg);
            width: 68px;
            height: 68px;
        }
        .circle-wrap .bg {
            fill: none;
            stroke: var(--border-subtle);
            stroke-width: 5;
            transition: var(--transition-smooth);
        }
        .circle-wrap .progress {
            fill: none;
            stroke: var(--green-bright);
            stroke-width: 5;
            stroke-linecap: round;
            transition: stroke-dashoffset 0.35s cubic-bezier(0.34, 1.56, 0.64, 1);
            filter: drop-shadow(0 0 6px rgba(46, 160, 67, 0.15));
        }
        .pod.active .circle-wrap .progress {
            filter: drop-shadow(0 0 12px rgba(46, 160, 67, 0.25));
        }
        .circle-wrap .center {
            position: absolute;
            inset: 0;
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
            font-size: 13px;
            font-weight: 700;
            font-variant-numeric: tabular-nums;
            color: var(--text-primary);
            line-height: 1.2;
        }
        .circle-wrap .center .label {
            font-size: 7px;
            font-weight: 600;
            text-transform: uppercase;
            color: var(--text-secondary);
            letter-spacing: 0.5px;
            margin-top: 2px;
            transition: var(--transition-smooth);
        }
        .pod.active .center .label {
            color: var(--text-primary);
        }

        .pod-inputs {
            display: flex;
            gap: 6px;
            margin: 6px 0 4px;
            justify-content: center;
        }
        .pod-input-group {
            display: flex;
            align-items: center;
            background: var(--bg-card);
            border-radius: 4px;
            border: 1px solid var(--border-subtle);
            padding: 0 4px;
            flex: 1;
            transition: var(--transition-smooth);
        }
        .pod-input-group:focus-within {
            border-color: var(--green-dim);
            box-shadow: 0 0 12px rgba(46, 160, 67, 0.03);
        }
        .pod-input-group .unit-label {
            font-size: 8px;
            font-weight: 700;
            color: var(--text-muted);
            text-transform: uppercase;
            padding: 0 2px;
        }
        .pod-input-group input {
            width: 100%;
            padding: 6px 0;
            background: transparent;
            border: none;
            color: var(--text-primary);
            font-size: 12px;
            font-weight: 600;
            text-align: center;
            outline: none;
            font-family: var(--font);
        }

        .pod-actions {
            display: flex;
            gap: 4px;
            margin-top: 4px;
        }
        .pod-actions button {
            flex: 1;
            padding: 6px 0;
            border: none;
            border-radius: 6px;
            font-size: 10px;
            font-weight: 700;
            text-transform: uppercase;
            letter-spacing: 0.5px;
            cursor: pointer;
            transition: var(--transition-butter);
            background: transparent;
            color: var(--text-secondary);
            border: 1px solid var(--border-subtle);
            font-family: var(--font);
        }
        .pod-actions .start {
            background: rgba(46, 160, 67, 0.06);
            color: var(--green-bright);
            border-color: rgba(46, 160, 67, 0.08);
        }
        .pod-actions .start:active {
            transform: scale(0.94);
            background: rgba(46, 160, 67, 0.12);
            box-shadow: 0 0 20px rgba(46, 160, 67, 0.05);
        }
        .pod-actions .cancel {
            color: var(--text-muted);
        }
        .pod-actions .cancel:active {
            transform: scale(0.94);
            background: rgba(255, 255, 255, 0.02);
        }

        input[type="number"]::-webkit-inner-spin-button {
            opacity: 0.3;
            transition: var(--transition-smooth);
        }
        input[type="number"] {
            -moz-appearance: textfield;
        }
    </style>
</head>
<body>

    <div class="app">

        <header class="header">
            <div class="header-left">
                <button class="btn-settings" id="settingsToggle" aria-label="Settings">&#9881;</button>
                <h1>Smart Automation</h1>
            </div>
            <div class="header-right">
                <span class="clock" id="clockDisplay">--:--:--</span>
            </div>
        </header>

        <div class="card" id="cardFan">
            <div class="card-header">
                <span class="device-title">
                    <svg class="icon" viewBox="0 0 24 24" stroke-linecap="round" stroke-linejoin="round">
                        <path d="M12 2a10 10 0 0 0 0 20 10 10 0 0 0 0-20z" />
                        <path d="M12 8v8M8 12h8" />
                        <path d="M16.5 7.5l-9 9" />
                    </svg>
                    Fan
                </span>
                <span class="badge" id="srcFan">idle</span>
            </div>

            <div class="toggle-row">
                <span class="toggle-label">Power</span>
                <label class="toggle">
                    <input type="checkbox" id="swFan">
                    <span class="slider"></span>
                </label>
            </div>

            <div class="schedule-section">
                <div class="schedule-header">
                    <span class="section-label">Daily Schedule</span>
                    <label class="toggle-mini">
                        <input type="checkbox" id="schEnFan">
                        <span class="slider-mini"></span>
                    </label>
                </div>
                <div class="schedule-row">
                    <div class="time-group">
                        <span class="time-label">ON</span>
                        <input type="number" placeholder="Hr" min="1" max="12" value="6" id="schOnHFan">
                        <span class="colon">:</span>
                        <input type="number" placeholder="Min" min="0" max="59" value="0" id="schOnMFan">
                        <select id="schOnAPFan"><option value="AM">AM</option><option value="PM" selected>PM</option></select>
                    </div>
                    <div class="time-group">
                        <span class="time-label">OFF</span>
                        <input type="number" placeholder="Hr" min="1" max="12" value="10" id="schOffHFan">
                        <span class="colon">:</span>
                        <input type="number" placeholder="Min" min="0" max="59" value="0" id="schOffMFan">
                        <select id="schOffAPFan"><option value="AM">AM</option><option value="PM" selected>PM</option></select>
                    </div>
                    <button class="btn-save-schedule" onclick="saveSchedule('fan')">Save</button>
                </div>
                <div class="schedule-feedback" id="schFeedbackFan"></div>
            </div>

            <div class="timer-grid">
                <div class="pod" id="aoPodFan">
                    <div class="pod-title">Auto ON</div>
                    <div class="circle-wrap">
                        <svg viewBox="0 0 68 68"><circle class="bg" cx="34" cy="34" r="29" /><circle class="progress" id="aoCircleFan" cx="34" cy="34" r="29" stroke-dasharray="182.2" stroke-dashoffset="182.2" /></svg>
                        <div class="center"><span id="aoTimeFan" style="color:var(--text-secondary);">--</span><span class="label">idle</span></div>
                    </div>
                    <div class="pod-inputs">
                        <div class="pod-input-group"><span class="unit-label">Hrs</span><input type="number" placeholder="0" min="0" value="0" id="aoHfan"></div>
                        <div class="pod-input-group"><span class="unit-label">Mins</span><input type="number" placeholder="0" min="0" value="5" id="aoMfan"></div>
                    </div>
                    <div class="pod-actions"><button class="start" id="aoStartFan">Start</button><button class="cancel" id="aoCancelFan">Cancel</button></div>
                </div>
                <div class="pod" id="ofPodFan">
                    <div class="pod-title">Auto OFF</div>
                    <div class="circle-wrap">
                        <svg viewBox="0 0 68 68"><circle class="bg" cx="34" cy="34" r="29" /><circle class="progress" id="ofCircleFan" cx="34" cy="34" r="29" stroke-dasharray="182.2" stroke-dashoffset="182.2" /></svg>
                        <div class="center"><span id="ofTimeFan" style="color:var(--text-secondary);">--</span><span class="label">idle</span></div>
                    </div>
                    <div class="pod-inputs">
                        <div class="pod-input-group"><span class="unit-label">Hrs</span><input type="number" placeholder="0" min="0" value="0" id="ofHfan"></div>
                        <div class="pod-input-group"><span class="unit-label">Mins</span><input type="number" placeholder="0" min="0" value="5" id="ofMfan"></div>
                    </div>
                    <div class="pod-actions"><button class="start" id="ofStartFan">Start</button><button class="cancel" id="ofCancelFan">Cancel</button></div>
                </div>
            </div>
        </div>

        <div class="card" id="cardLed">
            <div class="card-header">
                <span class="device-title">
                    <svg class="icon" viewBox="0 0 24 24" stroke-linecap="round" stroke-linejoin="round">
                        <circle cx="12" cy="12" r="10" /><path d="M12 6v6l4 2" />
                    </svg>
                    LED Bulb
                </span>
                <span class="badge" id="srcLed">idle</span>
            </div>

            <div class="toggle-row">
                <span class="toggle-label">Power</span>
                <label class="toggle">
                    <input type="checkbox" id="swLed">
                    <span class="slider"></span>
                </label>
            </div>

            <div class="schedule-section">
                <div class="schedule-header">
                    <span class="section-label">Daily Schedule</span>
                    <label class="toggle-mini">
                        <input type="checkbox" id="schEnLed">
                        <span class="slider-mini"></span>
                    </label>
                </div>
                <div class="schedule-row">
                    <div class="time-group">
                        <span class="time-label">ON</span>
                        <input type="number" placeholder="Hr" min="1" max="12" value="7" id="schOnHLed">
                        <span class="colon">:</span>
                        <input type="number" placeholder="Min" min="0" max="59" value="0" id="schOnMLed">
                        <select id="schOnAPLed"><option value="AM" selected>AM</option><option value="PM">PM</option></select>
                    </div>
                    <div class="time-group">
                        <span class="time-label">OFF</span>
                        <input type="number" placeholder="Hr" min="1" max="12" value="11" id="schOffHLed">
                        <span class="colon">:</span>
                        <input type="number" placeholder="Min" min="0" max="59" value="0" id="schOffMLed">
                        <select id="schOffAPLed"><option value="AM">AM</option><option value="PM" selected>PM</option></select>
                    </div>
                    <button class="btn-save-schedule" onclick="saveSchedule('led')">Save</button>
                </div>
                <div class="schedule-feedback" id="schFeedbackLed"></div>
            </div>

            <div class="timer-grid">
                <div class="pod" id="aoPodLed">
                    <div class="pod-title">Auto ON</div>
                    <div class="circle-wrap">
                        <svg viewBox="0 0 68 68"><circle class="bg" cx="34" cy="34" r="29" /><circle class="progress" id="aoCircleLed" cx="34" cy="34" r="29" stroke-dasharray="182.2" stroke-dashoffset="182.2" /></svg>
                        <div class="center"><span id="aoTimeLed" style="color:var(--text-secondary);">--</span><span class="label">idle</span></div>
                    </div>
                    <div class="pod-inputs">
                        <div class="pod-input-group"><span class="unit-label">Hrs</span><input type="number" placeholder="0" min="0" value="0" id="aoHLed"></div>
                        <div class="pod-input-group"><span class="unit-label">Mins</span><input type="number" placeholder="0" min="0" value="10" id="aoMLed"></div>
                    </div>
                    <div class="pod-actions"><button class="start" id="aoStartLed">Start</button><button class="cancel" id="aoCancelLed">Cancel</button></div>
                </div>
                <div class="pod" id="ofPodLed">
                    <div class="pod-title">Auto OFF</div>
                    <div class="circle-wrap">
                        <svg viewBox="0 0 68 68"><circle class="bg" cx="34" cy="34" r="29" /><circle class="progress" id="ofCircleLed" cx="34" cy="34" r="29" stroke-dasharray="182.2" stroke-dashoffset="182.2" /></svg>
                        <div class="center"><span id="ofTimeLed" style="color:var(--text-secondary);">--</span><span class="label">idle</span></div>
                    </div>
                    <div class="pod-inputs">
                        <div class="pod-input-group"><span class="unit-label">Hrs</span><input type="number" placeholder="0" min="0" value="0" id="ofHLed"></div>
                        <div class="pod-input-group"><span class="unit-label">Mins</span><input type="number" placeholder="0" min="0" value="30" id="ofMLed"></div>
                    </div>
                    <div class="pod-actions"><button class="start" id="ofStartLed">Start</button><button class="cancel" id="ofCancelLed">Cancel</button></div>
                </div>
            </div>
        </div>

        <div class="settings-overlay" id="settingsOverlay"></div>

        <div class="settings-panel" id="settingsPanel">
            <div class="panel-header">
                <div class="title-area">
                    <button class="back-btn" id="settingsBack">&#8592;</button>
                    <h2 id="settingsTitle">Settings</h2>
                </div>
                <button class="close-btn" id="settingsClose">&#10005;</button>
            </div>

            <div class="settings-content" id="settingsContent">

                <div class="settings-page" id="pageMenu">
                    <div class="menu-list">
                        <div class="menu-item" onclick="navigateTo('pageOTA')">
                            <div>
                                <div>Firmware Update</div>
                                <div class="desc">Over-The-Air (OTA) upgrade</div>
                            </div>
                            <span class="arrow">&#8250;</span>
                        </div>
                        <div class="menu-item" onclick="navigateTo('pageAP')">
                            <div>
                                <div>Access Point Setup</div>
                                <div class="desc">Change ESP32 setup WiFi name / password</div>
                            </div>
                            <span class="arrow">&#8250;</span>
                        </div>
                        <div class="menu-item" onclick="navigateTo('pageWiFi')">
                            <div>
                                <div>WiFi Client (Station)</div>
                                <div class="desc">Scan &amp; connect to home/router WiFi</div>
                            </div>
                            <span class="arrow">&#8250;</span>
                        </div>
                    </div>
                </div>

                <div class="settings-page slide-in" id="pageOTA">
                    <div class="sub-group">
                        <label>Choose Firmware File</label>
                        <input type="file" accept=".bin" class="file-inp" id="fwFile">
                        <div class="btn-row" style="margin-top:10px;">
                            <button class="btn btn-primary" id="flashBtn">Flash Firmware</button>
                        </div>
                        <div class="wifi-status-text" id="otaStat"></div>
                    </div>
                    <button class="btn-back-bottom" onclick="navigateTo('pageMenu')">Back to Menu</button>
                </div>

                <div class="settings-page slide-in" id="pageAP">
                    <div class="sub-group">
                        <label>Access Point SSID</label>
                        <input class="inp" placeholder="AP SSID" id="apSsid">
                    </div>
                    <div class="sub-group">
                        <label>Access Point Password (blank = open, or 8+ chars)</label>
                        <input class="inp" placeholder="AP Password" type="text" id="apPass">
                    </div>
                    <div class="btn-row">
                        <button class="btn btn-primary" id="apSaveBtn">Save &amp; Restart</button>
                        <button class="btn btn-danger" id="factoryResetBtn">Reset</button>
                    </div>
                    <button class="btn-back-bottom" onclick="navigateTo('pageMenu')">Back to Menu</button>
                </div>

                <div class="settings-page slide-in" id="pageWiFi">
                    <div class="sub-group">
                        <label>WiFi Networks</label>
                        <div class="btn-row" style="margin-bottom: 8px;">
                            <button class="btn btn-primary" id="scanWiFiBtn">Scan Networks</button>
                        </div>
                        <div id="wifiListContainer">
                            <div class="wifi-status-text" id="wifiStatusText">Press "Scan" to discover networks.</div>
                            <div class="wifi-list" id="wifiList"></div>
                        </div>
                    </div>
                    <button class="btn-back-bottom" onclick="navigateTo('pageMenu')">Back to Menu</button>
                </div>
            </div>
        </div>

    </div>

    <script>
        var API_KEY = "%API_KEY%";
        var OTA_USER = "admin";

        var CIRCUM = 182.2;
        var ws;
        var wsAlive = false;
        var suppressUntil = { fan: 0, led: 0 };

        var settingsToggle = document.getElementById('settingsToggle');
        var settingsPanel = document.getElementById('settingsPanel');
        var settingsOverlay = document.getElementById('settingsOverlay');
        var settingsClose = document.getElementById('settingsClose');
        var settingsBack = document.getElementById('settingsBack');
        var settingsTitle = document.getElementById('settingsTitle');

        var pageMenu = document.getElementById('pageMenu');
        var pageOTA = document.getElementById('pageOTA');
        var pageAP = document.getElementById('pageAP');
        var pageWiFi = document.getElementById('pageWiFi');
        var currentPage = 'pageMenu';

        function showPage(id) {
            var pages = [pageMenu, pageOTA, pageAP, pageWiFi];
            for (var i = 0; i < pages.length; i++) {
                pages[i].classList.remove('active', 'slide-in', 'slide-out');
                pages[i].style.display = 'none';
            }
            var target = document.getElementById(id);
            if (target) {
                target.style.display = 'block';
                target.classList.add('active');
            }
            var titles = {
                'pageMenu': 'Settings',
                'pageOTA': 'Firmware Update',
                'pageAP': 'Access Point',
                'pageWiFi': 'WiFi Client'
            };
            settingsTitle.textContent = titles[id] || 'Settings';
            if (id === 'pageMenu') {
                settingsBack.classList.remove('show');
                settingsBack.style.display = 'none';
            } else {
                settingsBack.classList.add('show');
                settingsBack.style.display = 'block';
            }
            currentPage = id;
        }

        function navigateTo(id) {
            if (id === 'pageMenu') {
                showPage('pageMenu');
                if (currentPage === 'pageWiFi') {
                    document.getElementById('wifiList').innerHTML = '';
                    document.getElementById('wifiStatusText').textContent = 'Press "Scan" to discover networks.';
                }
                return;
            }
            showPage(id);
        }

        settingsBack.addEventListener('click', function () { navigateTo('pageMenu'); });

        function openSettings() {
            settingsPanel.classList.add('open');
            settingsOverlay.classList.add('open');
            navigateTo('pageMenu');
        }
        function closeSettings() {
            settingsPanel.classList.remove('open');
            settingsOverlay.classList.remove('open');
        }
        settingsToggle.addEventListener('click', openSettings);
        settingsClose.addEventListener('click', closeSettings);
        settingsOverlay.addEventListener('click', closeSettings);

        function pad(n) { return n < 10 ? '0' + n : '' + n; }

        function fmtTime(sec) {
            if (sec < 0) sec = 0;
            var h = Math.floor(sec / 3600);
            var m = Math.floor((sec % 3600) / 60);
            var s = Math.floor(sec % 60);
            if (h > 0) return pad(h) + ':' + pad(m) + ':' + pad(s);
            return pad(m) + ':' + pad(s);
        }

        function fmtClock24(h, m) { return pad(h) + ':' + pad(m); }

        function to24(h, m, ap) {
            h = parseInt(h, 10);
            m = parseInt(m, 10);
            if (ap === 'PM' && h !== 12) h += 12;
            if (ap === 'AM' && h === 12) h = 0;
            return { h: h, m: m };
        }

        function to12(h) {
            var ap = (h >= 12) ? 'PM' : 'AM';
            var h12 = h % 12;
            if (h12 === 0) h12 = 12;
            return { h: h12, ap: ap };
        }

        function isFieldFocused(ids) {
            var active = document.activeElement;
            if (!active) return false;
            for (var i = 0; i < ids.length; i++) {
                if (active.id === ids[i]) return true;
            }
            return false;
        }

        function updateCircle(el, remain, total) {
            if (total <= 0) total = 1;
            var frac = Math.max(0, Math.min(1, remain / total));
            el.setAttribute('stroke-dashoffset', CIRCUM * (1 - frac));
        }

        function apiPost(endpoint, data, callback, errCallback) {
            var xhr = new XMLHttpRequest();
            xhr.open('POST', endpoint, true);
            xhr.setRequestHeader('Content-Type', 'application/json');
            xhr.setRequestHeader('X-API-Key', API_KEY);
            xhr.onload = function () {
                var resp = null;
                try { resp = JSON.parse(xhr.responseText); } catch (e) {}
                if (xhr.status >= 200 && xhr.status < 300) {
                    if (callback) callback(resp);
                } else if (errCallback) {
                    errCallback(resp, xhr.status);
                }
            };
            xhr.onerror = function () { if (errCallback) errCallback(null, 0); };
            xhr.send(JSON.stringify(data));
        }

        function apiGet(endpoint, callback, errCallback) {
            var xhr = new XMLHttpRequest();
            xhr.open('GET', endpoint, true);
            xhr.setRequestHeader('X-API-Key', API_KEY);
            xhr.onload = function () {
                if (xhr.status >= 200 && xhr.status < 300) {
                    var resp = null;
                    try { resp = JSON.parse(xhr.responseText); } catch (e) {}
                    if (callback) callback(resp);
                } else if (errCallback) {
                    errCallback(xhr.status);
                }
            };
            xhr.onerror = function () { if (errCallback) errCallback(0); };
            xhr.send();
        }

        function connectWS() {
            ws = new WebSocket('ws://' + location.host + '/ws');
            ws.onopen = function () { wsAlive = true; };
            ws.onclose = function () { wsAlive = false; setTimeout(connectWS, 2000); };
            ws.onerror = function () { ws.close(); };
            ws.onmessage = function (evt) {
                try { applyStatus(JSON.parse(evt.data)); } catch (e) {}
            };
        }

        function pollStatus() {
            apiGet('/api/status', applyStatus);
        }

        function renderDevice(dev, prefix) {
            var swEl = document.getElementById('sw' + prefix);
            var cardEl = document.getElementById('card' + prefix);
            var srcEl = document.getElementById('src' + prefix);
            var key = prefix.toLowerCase();

            if (Date.now() > suppressUntil[key]) {
                swEl.checked = dev.state;
            }
            if (dev.state) {
                cardEl.classList.add('active');
                srcEl.textContent = 'active';
            } else {
                cardEl.classList.remove('active');
                srcEl.textContent = 'idle';
            }

            var onHId = 'schOnH' + prefix, onMId = 'schOnM' + prefix, onAPId = 'schOnAP' + prefix;
            var offHId = 'schOffH' + prefix, offMId = 'schOffM' + prefix, offAPId = 'schOffAP' + prefix;
            var scheduleFieldIds = [onHId, onMId, onAPId, offHId, offMId, offAPId];

            if (!isFieldFocused(scheduleFieldIds)) {
                document.getElementById('schEn' + prefix).checked = dev.schedule.enabled;
                var on12 = to12(dev.schedule.onHour);
                var off12 = to12(dev.schedule.offHour);
                document.getElementById(onHId).value = on12.h;
                document.getElementById(onAPId).value = on12.ap;
                document.getElementById(onMId).value = pad(dev.schedule.onMin);
                document.getElementById(offHId).value = off12.h;
                document.getElementById(offAPId).value = off12.ap;
                document.getElementById(offMId).value = pad(dev.schedule.offMin);
            }

            var fb = document.getElementById('schFeedback' + prefix);
            if (dev.schedule.enabled) {
                var onT = to12(dev.schedule.onHour), offT = to12(dev.schedule.offHour);
                fb.textContent = 'Schedule active: ' + pad(onT.h) + ':' + pad(dev.schedule.onMin) + ' ' + onT.ap +
                    ' -> ' + pad(offT.h) + ':' + pad(dev.schedule.offMin) + ' ' + offT.ap;
                fb.className = 'schedule-feedback ok';
            } else {
                fb.textContent = 'Schedule disabled';
                fb.className = 'schedule-feedback';
            }

            renderPod('aoPod' + prefix, 'aoCircle' + prefix, 'aoTime' + prefix, dev.autoOn);
            renderPod('ofPod' + prefix, 'ofCircle' + prefix, 'ofTime' + prefix, dev.autoOff);
        }

        function renderPod(podId, circleId, timeId, timerObj) {
            var pod = document.getElementById(podId);
            var circle = document.getElementById(circleId);
            var timeEl = document.getElementById(timeId);

            if (timerObj.active) {
                pod.classList.add('active');
                timeEl.textContent = fmtTime(timerObj.remainingSec);
                timeEl.style.color = '#f0f6fc';
                updateCircle(circle, timerObj.remainingSec, timerObj.totalSec);
            } else {
                pod.classList.remove('active');
                timeEl.textContent = '--';
                timeEl.style.color = '#8b949e';
                updateCircle(circle, 0, 1);
            }
        }

        function applyStatus(s) {
            if (!s || !s.devices) return;
            document.getElementById('clockDisplay').textContent = s.time;
            renderDevice(s.devices.fan, 'Fan');
            renderDevice(s.devices.led, 'Led');
        }

        function toggle(device, state) {
            suppressUntil[device] = Date.now() + 3000;
            apiPost('/api/toggle', { device: device, state: state });
        }

        function saveSchedule(device) {
            var prefix = (device === 'fan') ? 'Fan' : 'Led';
            var en = document.getElementById('schEn' + prefix).checked;
            var onH = document.getElementById('schOnH' + prefix).value;
            var onM = document.getElementById('schOnM' + prefix).value;
            var onAP = document.getElementById('schOnAP' + prefix).value;
            var offH = document.getElementById('schOffH' + prefix).value;
            var offM = document.getElementById('schOffM' + prefix).value;
            var offAP = document.getElementById('schOffAP' + prefix).value;

            var on24 = to24(onH, onM, onAP);
            var off24 = to24(offH, offM, offAP);

            apiPost('/api/schedule', {
                device: device, enabled: en,
                onHour: on24.h, onMin: on24.m,
                offHour: off24.h, offMin: off24.m
            }, function () {
                pollStatus();
            });
        }

        document.getElementById('schEnFan').addEventListener('change', function () { saveSchedule('fan'); });
        document.getElementById('schEnLed').addEventListener('change', function () { saveSchedule('led'); });

        function startTimer(device, kind, hoursId, minsId) {
            var h = parseInt(document.getElementById(hoursId).value, 10) || 0;
            var m = parseInt(document.getElementById(minsId).value, 10) || 0;
            var minutes = h * 60 + m;
            if (minutes <= 0) { alert('Enter a valid duration'); return; }
            apiPost('/api/timer/start', { device: device, kind: kind, minutes: minutes }, pollStatus);
        }

        function cancelTimer(device, kind) {
            apiPost('/api/timer/cancel', { device: device, kind: kind }, pollStatus);
        }

        document.getElementById('aoStartFan').addEventListener('click', function () { startTimer('fan', 'on', 'aoHfan', 'aoMfan'); });
        document.getElementById('aoCancelFan').addEventListener('click', function () { cancelTimer('fan', 'on'); });
        document.getElementById('ofStartFan').addEventListener('click', function () { startTimer('fan', 'off', 'ofHfan', 'ofMfan'); });
        document.getElementById('ofCancelFan').addEventListener('click', function () { cancelTimer('fan', 'off'); });

        document.getElementById('aoStartLed').addEventListener('click', function () { startTimer('led', 'on', 'aoHLed', 'aoMLed'); });
        document.getElementById('aoCancelLed').addEventListener('click', function () { cancelTimer('led', 'on'); });
        document.getElementById('ofStartLed').addEventListener('click', function () { startTimer('led', 'off', 'ofHLed', 'ofMLed'); });
        document.getElementById('ofCancelLed').addEventListener('click', function () { cancelTimer('led', 'off'); });

        document.getElementById('swFan').addEventListener('change', function () { toggle('fan', this.checked); });
        document.getElementById('swLed').addEventListener('change', function () { toggle('led', this.checked); });

        document.getElementById('flashBtn').addEventListener('click', function () {
            var f = document.getElementById('fwFile').files[0];
            if (!f) { alert('Select a .bin file first'); return; }
            var pass = prompt('OTA password:');
            if (pass === null) return;

            var stat = document.getElementById('otaStat');
            var fd = new FormData();
            fd.append('firmware', f);

            var xhr = new XMLHttpRequest();
            xhr.open('POST', '/update');
            xhr.setRequestHeader('Authorization', 'Basic ' + btoa(OTA_USER + ':' + pass));
            xhr.upload.onprogress = function (e) {
                if (e.lengthComputable) stat.textContent = 'Uploading ' + Math.round(e.loaded / e.total * 100) + '%';
            };
            xhr.onload = function () {
                stat.textContent = (xhr.status === 200) ? 'Success - device is rebooting' : 'Failed: ' + xhr.responseText;
            };
            xhr.onerror = function () { stat.textContent = 'Upload error'; };
            xhr.send(fd);
        });

        document.getElementById('apSaveBtn').addEventListener('click', function () {
            var ssid = document.getElementById('apSsid').value;
            var pass = document.getElementById('apPass').value;
            if (!ssid) { alert('SSID required'); return; }
            if (pass.length > 0 && pass.length < 8) { alert('Password must be 8+ characters, or leave blank'); return; }
            if (!confirm('Device will restart. Continue?')) return;
            apiPost('/api/apconfig', { ssid: ssid, pass: pass }, function () {
                alert('Saved. Device is restarting.');
            }, function () {
                alert('Failed to save AP settings.');
            });
        });

        document.getElementById('factoryResetBtn').addEventListener('click', function () {
            if (!confirm('Reset ALL settings to default and restart?')) return;
            apiPost('/api/factoryreset', {}, function () {
                alert('Factory reset done. Device is restarting.');
            }, function () {
                alert('Factory reset failed.');
            });
        });

        function signalBars(rssi) {
            if (rssi >= -55) return '####';
            if (rssi >= -65) return '###.';
            if (rssi >= -75) return '##..';
            return '#...';
        }

        function scanWiFi() {
            var statusText = document.getElementById('wifiStatusText');
            var list = document.getElementById('wifiList');
            statusText.textContent = 'Scanning...';
            list.innerHTML = '';
            pollWifiScan(0);
        }

        function pollWifiScan(attempt) {
            var statusText = document.getElementById('wifiStatusText');
            var list = document.getElementById('wifiList');

            apiGet('/api/wifiscan', function (resp) {
                if (!resp) { statusText.textContent = 'Scan failed.'; return; }
                if (resp.status === 'scanning') {
                    if (attempt < 20) {
                        setTimeout(function () { pollWifiScan(attempt + 1); }, 500);
                    } else {
                        statusText.textContent = 'Scan timed out.';
                    }
                    return;
                }
                if (!resp.networks) {
                    statusText.textContent = 'Scan failed.';
                    return;
                }
                statusText.textContent = 'Select a network to connect:';
                var html = '<div class="wifi-item hidden-item" onclick="connectHidden()">' +
                    '+ Hidden Network (enter SSID)<span class="signal">Manual</span></div>';
                for (var i = 0; i < resp.networks.length; i++) {
                    var net = resp.networks[i];
                    html += '<div class="wifi-item" data-ssid="' + net.ssid + '" onclick="connectWiFi(this)">' +
                        net.ssid + '<span class="signal">' + signalBars(net.rssi) + '</span></div>';
                }
                list.innerHTML = html;
            }, function () {
                statusText.textContent = 'Scan failed.';
            });
        }

        window.connectWiFi = function (el) {
            var ssid = el.getAttribute('data-ssid');
            var pass = prompt('Enter password for "' + ssid + '":');
            if (pass === null) return;
            doWifiConnect(ssid, pass);
        };

        window.connectHidden = function () {
            var ssid = prompt('Enter the hidden network SSID:');
            if (!ssid || ssid === '') { alert('SSID cannot be empty'); return; }
            var pass = prompt('Enter password for "' + ssid + '":');
            if (pass === null) return;
            doWifiConnect(ssid, pass);
        };

        function doWifiConnect(ssid, pass) {
            var statusText = document.getElementById('wifiStatusText');
            statusText.textContent = 'Connecting to ' + ssid + ' ...';
            apiPost('/api/wifi/connect', { ssid: ssid, pass: pass }, function () {
                pollWifiStatus(0);
            }, function () {
                statusText.textContent = 'Failed to start connection.';
            });
        }

        function pollWifiStatus(attempt) {
            var statusText = document.getElementById('wifiStatusText');
            apiGet('/api/wifi/status', function (resp) {
                if (!resp) { statusText.textContent = 'Status check failed.'; return; }
                if (resp.status === 'connected') {
                    statusText.textContent = 'Connected. New address: ' + resp.ip +
                        ' (or use http://smarthome.local)';
                } else if (resp.status === 'failed') {
                    statusText.textContent = 'Connection failed or timed out.';
                } else if (attempt < 40) {
                    setTimeout(function () { pollWifiStatus(attempt + 1); }, 500);
                } else {
                    statusText.textContent = 'No response - check the new network manually.';
                }
            }, function () {
                statusText.textContent = 'Status check failed.';
            });
        }

        document.getElementById('scanWiFiBtn').addEventListener('click', scanWiFi);

        connectWS();
        pollStatus();
        setInterval(pollStatus, 5000);
        setInterval(function () {
            var d = new Date();
            if (!wsAlive) {
                document.getElementById('clockDisplay').textContent =
                    pad(d.getHours()) + ':' + pad(d.getMinutes()) + ':' + pad(d.getSeconds());
            }
        }, 1000);
    </script>

</body>
</html>
)HTMLPAGE";

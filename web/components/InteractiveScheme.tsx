import React, { useState, useEffect, useRef } from 'react';

const arduinoColor = 'rgba(20, 40, 80, 0.55)';
const arduinoBorder = 'rgba(0, 207, 255, 0.7)';
const breadboardColor = 'rgba(224, 224, 224, 0.95)';
const breadboardBorder = '#888';
const espColor = 'rgba(40, 80, 120, 0.55)';
const picoColor = 'rgba(60, 200, 120, 0.45)';
const picoBorder = '#00cfff';
const wireColor = '#FF9800';
const wireColorSelected = '#FFEB3B';
const particleColor = '#00fff7';
const particleRadius = 6;
const particleCount = 3;
const animationDuration = 1800;

const arduinoWidth = 320;

const arduinoHeight = 440;
const breadboardWidth = 200;
const breadboardHeight = 260;
const espWidth = 230;
const espHeight = 400;
const picoWidth = 200;
const picoHeight = 340;


interface Position { x: number; y: number; }
interface Pin {
    label: string;
    x: number;
    y: number;
    side?: 'left' | 'right' | 'top' | 'bottom';
    color: string;
}


const arduinoPins: Pin[] = [

    ...Array.from({ length: 14 }, (_, i) => ({ label: `D${i}`, x: arduinoWidth, y: 58 + i * 27, side: 'right' as const, color: '#00cfff' })),

    ...Array.from({ length: 6 }, (_, i) => ({ label: `A${i}`, x: 0, y: 80 + i * 60, side: 'left' as const, color: '#00ffb7' })),

    { label: '5V', x: arduinoWidth / 2 + 100, y: 0, side: 'top' as const, color: '#ffeb3b' },
    { label: '3.3V', x: arduinoWidth / 2 + 50, y: 0, side: 'top' as const, color: '#ffeb3b' },
    { label: 'GND', x: arduinoWidth / 2 - 10, y: 0, side: 'top' as const, color: '#fff' },
    { label: 'Vin', x: arduinoWidth / 2 - 70, y: 0, side: 'top' as const, color: '#fff' },
    { label: 'RESET', x: arduinoWidth / 2 - 130, y: 0, side: 'top' as const, color: '#fff' },

    { label: 'MOSI', x: arduinoWidth, y: arduinoHeight - 135, side: 'right' as const, color: '#ff9800' },
    { label: 'MISO', x: arduinoWidth, y: arduinoHeight - 100, side: 'right' as const, color: '#ff9800' },
    { label: 'SCK', x: arduinoWidth, y: arduinoHeight - 65, side: 'right' as const, color: '#ff9800' },
    { label: 'SS', x: arduinoWidth, y: arduinoHeight - 30, side: 'right' as const, color: '#ff9800' },

    { label: 'SDA', x: 0, y: arduinoHeight - 135, side: 'left' as const, color: '#b0c9d9' },
    { label: 'SCL', x: 0, y: arduinoHeight - 65, side: 'left' as const, color: '#b0c9d9' },
];

// Fix ESP32 and Pico GPIO pin layout
// For ESP32, move the first 2 left-side GPIOs to the top
const espGpiosTop = [0, 2];
const espGpiosLeft = [4, 5, 12, 13];
const espGpiosBottom = [14, 15, 16, 17];
const espGpiosRight = [18, 19, 21, 22, 23, 25, 26, 27, 32, 33];
const espPins = [
    { label: '3V3', x: 0, y: 58, side: 'left' as const, color: '#ffeb3b' },
    { label: 'EN', x: 0, y: 104, side: 'left' as const, color: '#fff' },
    { label: 'GND', x: 0, y: 150, side: 'left' as const, color: '#fff' },
    { label: 'VIN', x: 0, y: 196, side: 'left' as const, color: '#fff' },
    ...espGpiosTop.map((n, i) => ({ label: `GPIO${n}`, x: 60 + i * 60, y: 0, side: 'top' as const, color: '#00cfff' })),
    ...espGpiosLeft.map((n, i) => ({ label: `GPIO${n}`, x: 0, y: 200 + i * 44, side: 'left' as const, color: '#00cfff' })),
    ...espGpiosRight.map((n, i) => ({ label: `GPIO${n}`, x: espWidth, y: 58 + i * 28, side: 'right' as const, color: '#00cfff' })),
    ...espGpiosBottom.map((n, i) => ({ label: `GPIO${n}`, x: 20 + i * 50, y: espHeight, side: 'bottom' as const, color: '#00cfff' })),
    { label: 'TX0', x: espWidth, y: espHeight - 118, side: 'right' as const, color: '#ff9800' },
    { label: 'RX0', x: espWidth, y: espHeight - 58, side: 'right' as const, color: '#ff9800' },
    { label: 'SDA', x: 0, y: espHeight - 118, side: 'left' as const, color: '#b0c9d9' },
    { label: 'SCL', x: 0, y: espHeight - 58, side: 'left' as const, color: '#b0c9d9' },
];
// For Pico, move GP13 to the top, spaced further from GP1
const picoGpiosTop = [0, 1, 13];
const picoGpiosLeft = [2, 3, 4, 5, 6, 7];
const picoGpiosBottom = [8, 9, 10, 11, 12];
const picoGpiosRight = [14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28];
const picoPins = [
    { label: '3V3', x: 0, y: 58, side: 'left' as const, color: '#ffeb3b' },
    { label: 'VSYS', x: 0, y: 100, side: 'left' as const, color: '#fff' },
    { label: 'GND', x: 0, y: 142, side: 'left' as const, color: '#fff' },
    { label: 'GP0', x: 40, y: 0, side: 'top' as const, color: '#00cfff' },
    { label: 'GP1', x: 90, y: 0, side: 'top' as const, color: '#00cfff' },
    { label: 'GP13', x: 170, y: 0, side: 'top' as const, color: '#00cfff' },
    ...picoGpiosLeft.map((n, i) => ({ label: `GP${n}`, x: 0, y: 130 + i * 34, side: 'left' as const, color: '#00cfff' })),
    ...picoGpiosRight.map((n, i) => ({ label: `GP${n}`, x: picoWidth, y: 58 + i * 18, side: 'right' as const, color: '#00cfff' })),
    ...picoGpiosBottom.map((n, i) => ({ label: `GP${n}`, x: 20 + i * 38, y: picoHeight, side: 'bottom' as const, color: '#00cfff' })),
    { label: 'ADC0', x: picoWidth, y: picoHeight - 118, side: 'right' as const, color: '#0f0' },
    { label: 'ADC1', x: picoWidth, y: picoHeight - 66, side: 'right' as const, color: '#0f0' },
    { label: 'ADC2', x: picoWidth, y: picoHeight - 18, side: 'right' as const, color: '#0f0' },
    { label: 'SDA', x: 0, y: picoHeight - 118, side: 'left' as const, color: '#b0c9d9' },
    { label: 'SCL', x: 0, y: picoHeight - 18, side: 'left' as const, color: '#b0c9d9' },
];

function lerp(a: number, b: number, t: number) {
    return a + (b - a) * t;
}

const InteractiveScheme: React.FC<{ resetLayout?: number }> = ({ resetLayout }) => {

    const [arduinoPos, setArduinoPos] = useState({ x: 120, y: 200 });
    const [breadboard1Pos, setBreadboard1Pos] = useState({ x: 480, y: 120 });
    const [esp1Pos, setEsp1Pos] = useState({ x: 750, y: 80 });
    const [esp2Pos, setEsp2Pos] = useState({ x: 750, y: 350 });
    const [breadboard2Pos, setBreadboard2Pos] = useState({ x: 1050, y: 220 });
    const [picoPos, setPicoPos] = useState({ x: 1320, y: 200 });
    const [sram1Pos, setSram1Pos] = useState({ x: 900, y: 180 });
    const [sram2Pos, setSram2Pos] = useState({ x: 900, y: 340 });
    const [eepromPos, setEepromPos] = useState({ x: 1150, y: 340 });
    const [drag, setDrag] = useState<{ type: string | null, offsetX: number, offsetY: number }>({ type: null, offsetX: 0, offsetY: 0 });
    const [time, setTime] = useState(Date.now());
    const svgRef = useRef<SVGSVGElement>(null);


    useEffect(() => {
        setArduinoPos({ x: -130, y: 200 });
        setBreadboard1Pos({ x: 190, y: 120 });
        setEsp1Pos({ x: 460, y: 80 });
        setEsp2Pos({ x: 460, y: 350 });
        setBreadboard2Pos({ x: 760, y: 220 });
        setPicoPos({ x: 1030, y: 200 });
        setSram1Pos({ x: 410, y: 180 });
        setSram2Pos({ x: 410, y: 340 });
        setEepromPos({ x: 860, y: 340 });
    }, [resetLayout]);


    const wires = [

        {
            id: 'wire1',
            from: () => ({ x: arduinoPos.x + arduinoWidth, y: arduinoPos.y + 15 }),
            to: () => ({ x: breadboard1Pos.x, y: breadboard1Pos.y + 20 }),
            connectedTo: ['arduino', 'breadboard1']
        },
        {
            id: 'wire2',
            from: () => ({ x: arduinoPos.x + arduinoWidth, y: arduinoPos.y + 45 }),
            to: () => ({ x: breadboard1Pos.x, y: breadboard1Pos.y + 80 }),
            connectedTo: ['arduino', 'breadboard1']
        },

        {
            id: 'wire3',
            from: () => ({ x: esp1Pos.x + espWidth, y: esp1Pos.y + 20 }),
            to: () => ({ x: breadboard2Pos.x, y: breadboard2Pos.y + 30 }),
            connectedTo: ['esp1', 'breadboard2']
        },

        {
            id: 'wire4',
            from: () => ({ x: esp2Pos.x + espWidth, y: esp2Pos.y + 30 }),
            to: () => ({ x: breadboard2Pos.x, y: breadboard2Pos.y + 70 }),
            connectedTo: ['esp2', 'breadboard2']
        },

        {
            id: 'wire5',
            from: () => ({ x: picoPos.x, y: picoPos.y + picoHeight / 2 }),
            to: () => ({ x: breadboard2Pos.x + breadboardWidth, y: breadboard2Pos.y + breadboardHeight / 2 }),
            connectedTo: ['pico', 'breadboard2']
        },
    ];


    const sramWidth = 70, sramHeight = 40;
    const eepromWidth = 70, eepromHeight = 40;





    function onMouseDown(type: string, e: React.MouseEvent) {
        const svgRect = svgRef.current?.getBoundingClientRect();
        let pos;
        switch (type) {
            case 'arduino': pos = arduinoPos; break;
            case 'breadboard1': pos = breadboard1Pos; break;
            case 'breadboard2': pos = breadboard2Pos; break;
            case 'esp1': pos = esp1Pos; break;
            case 'esp2': pos = esp2Pos; break;
            case 'pico': pos = picoPos; break;
            case 'sram1': pos = sram1Pos; break;
            case 'sram2': pos = sram2Pos; break;
            case 'eeprom': pos = eepromPos; break;
            default: pos = { x: 0, y: 0 };
        }
        const offsetX = e.clientX - pos.x - (svgRect?.left || 0);
        const offsetY = e.clientY - pos.y - (svgRect?.top || 0);
        setDrag({ type, offsetX, offsetY });
    }
    function onMouseMove(e: React.MouseEvent) {
        if (!drag.type) return;
        const svgRect = svgRef.current?.getBoundingClientRect();
        const x = e.clientX - (svgRect?.left || 0) - drag.offsetX;
        const y = e.clientY - (svgRect?.top || 0) - drag.offsetY;
        switch (drag.type) {
            case 'arduino': setArduinoPos({ x, y }); break;
            case 'breadboard1': setBreadboard1Pos({ x, y }); break;
            case 'breadboard2': setBreadboard2Pos({ x, y }); break;
            case 'esp1': setEsp1Pos({ x, y }); break;
            case 'esp2': setEsp2Pos({ x, y }); break;
            case 'pico': setPicoPos({ x, y }); break;
            case 'sram1': setSram1Pos({ x, y }); break;
            case 'sram2': setSram2Pos({ x, y }); break;
            case 'eeprom': setEepromPos({ x, y }); break;
        }
    }
    function onMouseUp() {
        setDrag({ type: null, offsetX: 0, offsetY: 0 });
    }


    function renderParticles(wire: typeof wires[0], idx: number) {
        const now = time % animationDuration;
        const particles = [];
        const from = wire.from();
        const to = wire.to();
        for (let i = 0; i < particleCount; i++) {
            const phase = ((now + (i * animationDuration) / particleCount) % animationDuration) / animationDuration;
            const t = phase;
            const x = lerp(from.x, to.x, t);
            const y = lerp(from.y, to.y, t);
            particles.push(
                <circle
                    key={i}
                    cx={x}
                    cy={y}
                    r={particleRadius}
                    fill={particleColor}
                    opacity={0.7}
                />
            );
        }
        return particles;
    }


    function renderBreadboard2Components() {
        const baseX = breadboard2Pos.x + 20;
        const baseY = breadboard2Pos.y + 40;
        const ledSpacing = 32;
        const ledCount = 4;
        const resistorLength = 22;
        const ledRadius = 7;
        const gndRailY = breadboard2Pos.y + breadboardHeight - 14 + 5;
        return (
            <g>
                {/* Green LEDs with resistors and GND wires */}
                {[0, 1, 2, 3].map(i => {
                    const x = baseX + 50 + i * ledSpacing;

                    return (
                        <g key={`g${i}`}>
                            <rect x={x - resistorLength - ledRadius - 2} y={baseY - 4} width={resistorLength} height={8} rx={3} fill="#e0b97a" stroke="#b8860b" strokeWidth={1.5} />
                            {/* Wire from resistor to LED anode */}
                            <line x1={x - ledRadius - 2} y1={baseY} x2={x - 2} y2={baseY} stroke="#888" strokeWidth={2} />
                            {/* LED */}
                            <circle cx={x} cy={baseY + 4} r={ledRadius} fill="#0f0" stroke="#222" strokeWidth={2} />
                            {/* Wire from LED cathode to GND rail */}
                            <line x1={x + ledRadius} y1={baseY + 4} x2={x + ledRadius} y2={gndRailY} stroke="#2196f3" strokeWidth={2} />
                        </g>
                    );
                })}
                {/* Red LEDs with resistors and GND wires */}
                {[0, 1, 2, 3].map(i => {
                    const x = baseX + 50 + i * ledSpacing;

                    return (
                        <g key={`r${i}`}>
                            <rect x={x - resistorLength - ledRadius - 2} y={baseY + 30} width={resistorLength} height={8} rx={3} fill="#e0b97a" stroke="#b8860b" strokeWidth={1.5} />
                            {/* Wire from resistor to LED anode */}
                            <line x1={x - ledRadius - 2} y1={baseY + 34} x2={x - 2} y2={baseY + 34} stroke="#888" strokeWidth={2} />
                            {/* LED */}
                            <circle cx={x} cy={baseY + 34} r={ledRadius} fill="#f00" stroke="#222" strokeWidth={2} />
                            {/* Wire from LED cathode to GND rail */}
                            <line x1={x + ledRadius} y1={baseY + 34} x2={x + ledRadius} y2={gndRailY} stroke="#2196f3" strokeWidth={2} />
                        </g>
                    );
                })}
            </g>
        );
    }


    function renderBreadboardLook(pos: Position) {

        const railColor = '#e0e0e0';
        const holeColor = '#bbb';
        const railW = breadboardWidth - 16;
        const railH = 10;
        const holes = [];

        for (let row = 0; row < 4; row++) {
            for (let i = 0; i < 30; i++) {
                holes.push(
                    <circle
                        key={`h${row}-${i}`}
                        cx={pos.x + 12 + i * 6}
                        cy={pos.y + 24 + row * 32}
                        r={2.2}
                        fill={holeColor}
                        stroke="#888"
                        strokeWidth={0.5}
                    />
                );
            }
        }
        return (
            <g>
                {/* Power rails */}
                <rect x={pos.x + 4} y={pos.y + 4} width={railW} height={railH} rx={4} fill={railColor} stroke="#f00" strokeWidth={2} />
                <rect x={pos.x + 4} y={pos.y + breadboardHeight - 14} width={railW} height={railH} rx={4} fill={railColor} stroke="#2196f3" strokeWidth={2} />
                {/* + and - labels */}
                <text x={pos.x + 8} y={pos.y + 14} fontSize="14" fill="#f00" fontWeight="bold">+</text>
                <text x={pos.x + 8} y={pos.y + breadboardHeight - 6} fontSize="14" fill="#2196f3" fontWeight="bold">-</text>
                {/* Main board holes */}
                {holes}
            </g>
        );
    }

    // Update renderPins to support 'bottom' side
    function renderPins(pins: Pin[], pos: Position, width: number, height: number) {
        return pins.map((pin: Pin, i: number) => (
            <g key={i}>
                <circle
                    cx={pos.x + (pin.side === 'left' ? 0 : pin.side === 'right' ? width : pin.side === 'top' ? pin.x : pin.side === 'bottom' ? pin.x : pin.x)}
                    cy={pos.y + (pin.side === 'top' ? 0 : pin.side === 'bottom' ? height : pin.y)}
                    r={5}
                    fill={pin.color}
                    stroke="#222"
                    strokeWidth={1}
                    opacity={0.85}
                />
                <text
                    x={pos.x + (pin.side === 'left' ? -12 : pin.side === 'right' ? width + 12 : pin.side === 'top' ? pin.x : pin.side === 'bottom' ? pin.x : pin.x + 18)}
                    y={pos.y + (pin.side === 'top' ? -6 : pin.side === 'bottom' ? height + 18 : pin.y + 4)}
                    textAnchor={pin.side === 'left' ? 'end' : pin.side === 'right' ? 'start' : 'middle'}
                    fill="#fff"
                    fontSize="11"
                    fontWeight="bold"
                    style={{ fontFamily: 'Orbitron, Arial, sans-serif', textShadow: '0 0 2px #00cfff' }}
                >
                    {pin.label}
                </text>
            </g>
        ));
    }


    const extraWires: { from: () => { x: number; y: number }, to: () => { x: number; y: number }, color: string }[] = [

        { from: () => ({ x: breadboard2Pos.x + 10, y: breadboard2Pos.y + 9 }), to: () => ({ x: esp1Pos.x, y: esp1Pos.y + 38 }), color: '#f00' },
        { from: () => ({ x: breadboard2Pos.x + 10, y: breadboard2Pos.y + 9 }), to: () => ({ x: esp2Pos.x, y: esp2Pos.y + 38 }), color: '#f00' },
        { from: () => ({ x: breadboard2Pos.x + 10, y: breadboard2Pos.y + 9 }), to: () => ({ x: picoPos.x, y: picoPos.y + 38 }), color: '#f00' },

        { from: () => ({ x: breadboard2Pos.x + 10, y: breadboard2Pos.y + breadboardHeight - 9 }), to: () => ({ x: esp1Pos.x, y: esp1Pos.y + 130 }), color: '#2196f3' },
        { from: () => ({ x: breadboard2Pos.x + 10, y: breadboard2Pos.y + breadboardHeight - 9 }), to: () => ({ x: esp2Pos.x, y: esp2Pos.y + 130 }), color: '#2196f3' },
        { from: () => ({ x: breadboard2Pos.x + 10, y: breadboard2Pos.y + breadboardHeight - 9 }), to: () => ({ x: picoPos.x, y: picoPos.y + 152 }), color: '#2196f3' },

        { from: () => ({ x: esp1Pos.x + espWidth, y: esp1Pos.y + 60 }), to: () => ({ x: sram1Pos.x, y: sram1Pos.y + sramHeight / 2 }), color: '#8e24aa' },

        { from: () => ({ x: esp2Pos.x + espWidth, y: esp2Pos.y + 60 }), to: () => ({ x: sram2Pos.x, y: sram2Pos.y + sramHeight / 2 }), color: '#8e24aa' },

        { from: () => ({ x: sram1Pos.x + sramWidth, y: sram1Pos.y + sramHeight / 2 }), to: () => ({ x: eepromPos.x, y: eepromPos.y + eepromHeight / 2 - 10 }), color: '#009688' },

        { from: () => ({ x: sram2Pos.x + sramWidth, y: sram2Pos.y + sramHeight / 2 }), to: () => ({ x: eepromPos.x, y: eepromPos.y + eepromHeight / 2 + 10 }), color: '#009688' },

        { from: () => ({ x: eepromPos.x + eepromWidth, y: eepromPos.y + eepromHeight / 2 }), to: () => ({ x: breadboard2Pos.x + breadboardWidth / 2, y: breadboard2Pos.y + breadboardHeight / 2 }), color: '#ff9800' },
    ];

    return (
        <svg
            ref={svgRef}
            width="100vw"
            height="80vh"
            viewBox="0 0 1200 700"
            style={{ background: 'none', width: '100vw', height: '80vh', minHeight: 400, cursor: drag.type ? 'grabbing' : 'default', userSelect: 'none', display: 'block', zIndex: 10, position: 'relative' }}
            onMouseMove={onMouseMove}
            onMouseUp={onMouseUp}
            onMouseLeave={onMouseUp}
        >
            {/* Wires */}
            {wires.map(wire => {
                const from = wire.from();
                const to = wire.to();
                return (
                    <line
                        key={wire.id}
                        x1={from.x}
                        y1={from.y}
                        x2={to.x}
                        y2={to.y}
                        stroke={wireColor}
                        strokeWidth={2}
                        style={{ transition: 'stroke 0.2s, stroke-width 0.2s' }}
                    />
                );
            })}
            {/* Particles */}
            {wires.map((wire, idx) => renderParticles(wire, idx))}
            {/* Arduino (glass style, draggable) */}
            <g
                onMouseDown={e => onMouseDown('arduino', e)}
                style={{ cursor: 'grab' }}
            >
                <rect
                    x={arduinoPos.x}
                    y={arduinoPos.y}
                    width={arduinoWidth}
                    height={arduinoHeight}
                    rx={20}
                    fill={arduinoColor}
                    stroke={arduinoBorder}
                    strokeWidth={3}
                    style={{ filter: 'drop-shadow(0 0 16px #00cfff88) blur(0.5px)' }}
                />
                {/* Neon border effect */}
                <rect
                    x={arduinoPos.x + 3}
                    y={arduinoPos.y + 3}
                    width={arduinoWidth - 6}
                    height={arduinoHeight - 6}
                    rx={16}
                    fill="none"
                    stroke="#00cfff"
                    strokeWidth={1.5}
                    style={{ opacity: 0.5 }}
                />
                <text
                    x={arduinoPos.x + arduinoWidth / 2}
                    y={arduinoPos.y + arduinoHeight / 2 + 7}
                    textAnchor="middle"
                    fill="#e0f7fa"
                    fontSize="22"
                    fontWeight="bold"
                    style={{ fontFamily: 'Orbitron, Arial, sans-serif', textShadow: '0 0 8px #00cfff' }}
                >
                    Arduino
                </text>
                {renderPins(arduinoPins, arduinoPos, arduinoWidth, arduinoHeight)}
            </g>
            {/* Breadboard 1 (glass style, draggable) */}
            <g
                onMouseDown={e => onMouseDown('breadboard1', e)}
                style={{ cursor: 'grab' }}
            >
                <rect
                    x={breadboard1Pos.x}
                    y={breadboard1Pos.y}
                    width={breadboardWidth}
                    height={breadboardHeight}
                    rx={18}
                    fill={breadboardColor}
                    stroke={breadboardBorder}
                    strokeWidth={2}
                    style={{ filter: 'drop-shadow(0 0 12px #00cfff55)' }}
                />
                {/* Glassy effect */}
                <rect
                    x={breadboard1Pos.x + 2}
                    y={breadboard1Pos.y + 2}
                    width={breadboardWidth - 4}
                    height={breadboardHeight - 4}
                    rx={14}
                    fill="none"
                    stroke="#00cfff"
                    strokeWidth={1.2}
                    style={{ opacity: 0.3 }}
                />
                <text
                    x={breadboard1Pos.x + breadboardWidth / 2}
                    y={breadboard1Pos.y + breadboardHeight / 2 + 7}
                    textAnchor="middle"
                    fill="#232526"
                    fontSize="18"
                    fontWeight="bold"
                    style={{ fontFamily: 'Orbitron, Arial, sans-serif', textShadow: '0 0 6px #00cfff' }}
                >
                    Breadboard
                </text>
                {renderBreadboardLook(breadboard1Pos)}
            </g>
            <g
                onMouseDown={e => onMouseDown('breadboard2', e)}
                style={{ cursor: 'grab' }}
            >
                <rect
                    x={breadboard2Pos.x}
                    y={breadboard2Pos.y}
                    width={breadboardWidth}
                    height={breadboardHeight}
                    rx={18}
                    fill={breadboardColor}
                    stroke={breadboardBorder}
                    strokeWidth={2}
                    style={{ filter: 'drop-shadow(0 0 12px #00cfff55)' }}
                />
                <rect
                    x={breadboard2Pos.x + 2}
                    y={breadboard2Pos.y + 2}
                    width={breadboardWidth - 4}
                    height={breadboardHeight - 4}
                    rx={14}
                    fill="none"
                    stroke="#00cfff"
                    strokeWidth={1.2}
                    style={{ opacity: 0.3 }}
                />
                <text
                    x={breadboard2Pos.x + breadboardWidth / 2}
                    y={breadboard2Pos.y + breadboardHeight / 2 + 7}
                    textAnchor="middle"
                    fill="#232526"
                    fontSize="18"
                    fontWeight="bold"
                    style={{ fontFamily: 'Orbitron, Arial, sans-serif', textShadow: '0 0 6px #00cfff' }}
                >
                    Breadboard
                </text>
                {renderBreadboardLook(breadboard2Pos)}
                {renderBreadboard2Components()}
            </g>
            <g
                onMouseDown={e => onMouseDown('esp1', e)}
                style={{ cursor: 'grab' }}
            >
                <rect
                    x={esp1Pos.x}
                    y={esp1Pos.y}
                    width={espWidth}
                    height={espHeight}
                    rx={14}
                    fill={espColor}
                    stroke="#00cfff"
                    strokeWidth={2}
                    style={{ filter: 'drop-shadow(0 0 10px #00cfff88)' }}
                />
                <text
                    x={esp1Pos.x + espWidth / 2}
                    y={esp1Pos.y + espHeight / 2 + 6}
                    textAnchor="middle"
                    fill="#e0f7fa"
                    fontSize="18"
                    fontWeight="bold"
                    style={{ fontFamily: 'Orbitron, Arial, sans-serif', textShadow: '0 0 6px #00cfff' }}
                >
                    ESP32 #1
                </text>
                {renderPins(espPins, esp1Pos, espWidth, espHeight)}
            </g>
            <g
                onMouseDown={e => onMouseDown('esp2', e)}
                style={{ cursor: 'grab' }}
            >
                <rect
                    x={esp2Pos.x}
                    y={esp2Pos.y}
                    width={espWidth}
                    height={espHeight}
                    rx={14}
                    fill={espColor}
                    stroke="#00cfff"
                    strokeWidth={2}
                    style={{ filter: 'drop-shadow(0 0 10px #00cfff88)' }}
                />
                <text
                    x={esp2Pos.x + espWidth / 2}
                    y={esp2Pos.y + espHeight / 2 + 6}
                    textAnchor="middle"
                    fill="#e0f7fa"
                    fontSize="18"
                    fontWeight="bold"
                    style={{ fontFamily: 'Orbitron, Arial, sans-serif', textShadow: '0 0 6px #00cfff' }}
                >
                    ESP32 #2
                </text>
                {renderPins(espPins, esp2Pos, espWidth, espHeight)}
            </g>
            <g
                onMouseDown={e => onMouseDown('pico', e)}
                style={{ cursor: 'grab' }}
            >
                <rect
                    x={picoPos.x}
                    y={picoPos.y}
                    width={picoWidth}
                    height={picoHeight}
                    rx={10}
                    fill={picoColor}
                    stroke={picoBorder}
                    strokeWidth={2}
                    style={{ filter: 'drop-shadow(0 0 10px #00cfff88)' }}
                />
                <text
                    x={picoPos.x + picoWidth / 2}
                    y={picoPos.y + picoHeight / 2 + 6}
                    textAnchor="middle"
                    fill="#e0f7fa"
                    fontSize="16"
                    fontWeight="bold"
                    style={{ fontFamily: 'Orbitron, Arial, sans-serif', textShadow: '0 0 6px #00cfff' }}
                >
                    Pi Pico
                </text>
                {renderPins(picoPins, picoPos, picoWidth, picoHeight)}
            </g>
            <g onMouseDown={e => onMouseDown('sram1', e)} style={{ cursor: 'grab' }}>
                <rect x={sram1Pos.x} y={sram1Pos.y} width={sramWidth} height={sramHeight} rx={18}
                    fill="url(#sramGrad)" stroke="#8e24aa" strokeWidth={3}
                    style={{ filter: 'drop-shadow(0 0 24px #8e24aa)' }} />
                <text x={sram1Pos.x + sramWidth / 2} y={sram1Pos.y + sramHeight / 2 + 7} textAnchor="middle" fill="#fff" fontWeight="bold" fontSize="18" style={{ fontFamily: 'Orbitron, Arial, sans-serif', textShadow: '0 0 8px #8e24aa' }}>SRAM 1</text>
            </g>
            <g onMouseDown={e => onMouseDown('sram2', e)} style={{ cursor: 'grab' }}>
                <rect x={sram2Pos.x} y={sram2Pos.y} width={sramWidth} height={sramHeight} rx={18}
                    fill="url(#sramGrad)" stroke="#8e24aa" strokeWidth={3}
                    style={{ filter: 'drop-shadow(0 0 24px #8e24aa)' }} />
                <text x={sram2Pos.x + sramWidth / 2} y={sram2Pos.y + sramHeight / 2 + 7} textAnchor="middle" fill="#fff" fontWeight="bold" fontSize="18" style={{ fontFamily: 'Orbitron, Arial, sans-serif', textShadow: '0 0 8px #8e24aa' }}>SRAM 2</text>
            </g>
            <g onMouseDown={e => onMouseDown('eeprom', e)} style={{ cursor: 'grab' }}>
                <rect x={eepromPos.x} y={eepromPos.y} width={eepromWidth} height={eepromHeight} rx={18}
                    fill="url(#eepromGrad)" stroke="#ff9800" strokeWidth={3}
                    style={{ filter: 'drop-shadow(0 0 24px #ff9800)' }} />
                <text x={eepromPos.x + eepromWidth / 2} y={eepromPos.y + eepromHeight / 2 + 7} textAnchor="middle" fill="#fff" fontWeight="bold" fontSize="18" style={{ fontFamily: 'Orbitron, Arial, sans-serif', textShadow: '0 0 8px #ff9800' }}>EEPROM</text>
            </g>
            <defs>
                <linearGradient id="sramGrad" x1="0" y1="0" x2="1" y2="1">
                    <stop offset="0%" stopColor="#b388ff" />
                    <stop offset="100%" stopColor="#8e24aa" />
                </linearGradient>
                <linearGradient id="eepromGrad" x1="0" y1="0" x2="1" y2="1">
                    <stop offset="0%" stopColor="#ffe082" />
                    <stop offset="100%" stopColor="#ff9800" />
                </linearGradient>
            </defs>
            {extraWires.map((w: { from: () => { x: number; y: number }, to: () => { x: number; y: number }, color: string }, i: number) => {
                const from = w.from();
                const to = w.to();
                return (
                    <line key={i} x1={from.x} y1={from.y} x2={to.x} y2={to.y} stroke={w.color} strokeWidth={3} opacity={0.8} />
                );
            })}
        </svg>
    );
};

export default InteractiveScheme; 
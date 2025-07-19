import React, { useState } from 'react';
import Head from 'next/head';
import styles from '../styles/Home.module.css';
import InteractiveScheme from '../components/InteractiveScheme';

const Home: React.FC = () => {
    const [active, setActive] = useState(0);
    const [showScheme, setShowScheme] = useState(false);
    const [resetLayout, setResetLayout] = useState(0);

    const SECTIONS = [
        {
            key: 'concept',
            label: 'Concept',
            content: (
                <ul className={styles.dsList}>
                    <li className={styles.dsListItem}>CPU-like control and coordination across multiple microcontrollers</li>
                    <li className={styles.dsListItem}>Dynamic thermal management using onboard temperature sensors to control cooling fans</li>
                    <li className={styles.dsListItem}>LED arrays as system status indicators, boot-up animations, and diagnostics</li>
                    <li className={styles.dsListItem}>Parallel and serial shift registers for efficient expansion of digital I/O</li>
                    <li className={styles.dsListItem}>Mass storage integration with a microSD card module</li>
                    <li className={styles.dsListItem}>Visual feedback through a 4” SPI TFT display</li>
                    <li className={styles.dsListItem}>User inputs via DIP switches and buttons</li>
                    <li className={styles.dsListItem}>Modular power management supporting USB and battery sources</li>
                </ul>
            ),
        },
        {
            key: 'hardware',
            label: 'Hardware',
            content: (
                <ul className={styles.dsList}>
                    {[
                        { name: 'ESP32 Boards (x2)', role: 'Main “processing units” running complex LED control and managing communication' },
                        { name: 'Raspberry Pi Pico', role: '“Thermal control unit,” reading temperature sensors and triggering fans' },
                        { name: 'Arduino Nano/Uno', role: 'Handles auxiliary LED arrays and additional I/O expansion' },
                        { name: 'SN74HC165N/595N', role: 'Shift registers for scalable I/O, mimicking CPU buses' },
                        { name: 'MicroSD Card Module', role: 'Persistent storage for logs or data' },
                        { name: '5x 5V Fans', role: 'Controlled based on temperature thresholds for dynamic cooling' },
                        { name: '2.8” SPI TFT Display', role: 'Live visual feedback on system operation' },
                        { name: 'DIP Switch & Buttons', role: 'Manual user input, configuration, and debugging' },
                    ].map((item) => (
                        <li className={styles.dsListItem} key={item.name}><b>{item.name}:</b> {item.role}</li>
                    ))}
                </ul>
            ),
        },
        {
            key: 'features',
            label: 'Features',
            content: (
                <ul className={styles.dsList}>
                    {[
                        'Boot-up LED animations that light LEDs sequentially to mimic system initialization',
                        'Continuous LED status display for live monitoring',
                        'Fan speed control based on real-time temperature, automatically cooling hardware under load',
                        'Multi-core communication between microcontrollers simulating CPU and peripheral interactions',
                        'Flexible power input options (USB or battery)',
                        'Comprehensive debugging output and troubleshooting support via serial console',
                        'Expandable I/O using shift registers, allowing future additions without major rewiring',
                    ].map((f, i) => <li className={styles.dsListItem} key={i}>{f}</li>)}
                </ul>
            ),
        },
        {
            key: 'schemes',
            label: 'Schemes',
            content: (
                <div>
                    <p style={{ marginBottom: '1em' }}>Overview of board-to-component connections:</p>
                    <table className={styles.schemesTable}>
                        <thead>
                            <tr>
                                <th>Board</th>
                                <th>Connected Components</th>
                            </tr>
                        </thead>
                        <tbody>
                            <tr>
                                <td>Arduino Uno</td>
                                <td>1x Green LED, 1x Red LED, Relay, 2x SRAM, EEPROM (shared)</td>
                            </tr>
                            <tr>
                                <td>ESP32 #1</td>
                                <td>1x Green LED, 1x Red LED, Screen</td>
                            </tr>
                            <tr>
                                <td>ESP32 #2</td>
                                <td>1x Green LED, 1x Red LED, SD Card</td>
                            </tr>
                            <tr>
                                <td>SRAM (x2)</td>
                                <td>Each connected to an Arduino Uno</td>
                            </tr>
                            <tr>
                                <td>EEPROM</td>
                                <td>Shared by both Arduinos</td>
                            </tr>
                        </tbody>
                    </table>
                    <div style={{ textAlign: 'center', marginTop: '2em' }}>
                        <button
                            className={styles.schematicBtnAnimated}
                            onClick={() => { setShowScheme(true); setResetLayout(r => r + 1); }}
                        >
                            Show Interactive Schematic
                        </button>
                    </div>
                </div>
            ),
        },
        {
            key: 'usecases',
            label: 'Use Cases',
            content: (
                <ul className={styles.dsList}>
                    {[
                        'Educational tool for computer architecture and embedded systems',
                        'Prototype platform for testing multi-core communication',
                        'Custom DIY PC-like controller for small devices',
                        'Advanced LED and fan control system for electronics projects',
                        'Base for building more complex IoT or robotics systems',
                    ].map((u, i) => <li className={styles.dsListItem} key={i}>{u}</li>)}
                </ul>
            ),
        },
        {
            key: 'status',
            label: 'Status',
            content: (
                <ul className={styles.dsList}>
                    {[
                        'Hardware wired and tested across multiple breadboards',
                        'Core LED and fan control firmware running on ESP32, Pico, and Arduino',
                        'Ongoing work on enhanced communication protocols and storage integration',
                        'Documentation and wiring guides in progress',
                    ].map((s, i) => <li className={styles.dsListItem} key={i}>{s}</li>)}
                </ul>
            ),
        },
    ];

    return (
        <>
            <div className={styles.dsBgWrap}>
                <Head>
                    <title>MAYHEM Project</title>
                    <meta name="description" content="DIY Multi-Microcontroller PC Prototype" />
                    <link rel="icon" href="/favicon.ico" />
                </Head>
                <div className={styles.topbar}>
                    <span className={styles.topbarTitle}>MAYHEM</span>
                </div>
                <aside className={styles.sidebar}>
                    <nav className={styles.navMenu}>
                        {SECTIONS.map((section, idx) => (
                            <button
                                key={section.key}
                                className={styles.navBtn + (active === idx ? ' ' + styles.activeNav : '')}
                                onClick={() => setActive(idx)}
                                aria-label={section.label}
                            >
                                <span className={styles.navDot} />
                                {section.label}
                            </button>
                        ))}
                    </nav>
                </aside>
                <main className={styles.dsMain}>
                    <div className={styles.panelWrap}>
                        {SECTIONS.map((section, idx) => (
                            <section
                                key={section.key}
                                className={
                                    styles.glassPanel +
                                    ' ' + styles.floatingPanel +
                                    (active === idx ? ' ' + styles.panelActive : ' ' + styles.panelInactive)
                                }
                                style={{ zIndex: active === idx ? 2 : 1 }}
                                aria-hidden={active !== idx}
                            >
                                <h2 className={styles.sectionTitle}>{section.label}</h2>
                                <div className={styles.dsStrandThin} />
                                <div>{section.content}</div>
                            </section>
                        ))}
                    </div>
                </main>
                <div className={styles.scanlineVfx} />
                <div className={styles.particleLayer} />
            </div>
            {showScheme && (
                <div style={{
                    position: 'fixed',
                    top: 0,
                    left: 0,
                    width: '100vw',
                    height: '100vh',
                    background: 'rgba(10,20,40,0.97)',
                    zIndex: 1000,
                    display: 'flex',
                    flexDirection: 'column',
                    alignItems: 'center',
                    justifyContent: 'center',
                    transition: 'background 0.3s',
                }}>
                    <button
                        className={styles.schematicBtnAnimated}
                        onClick={() => setShowScheme(false)}
                        style={{
                            position: 'absolute',
                            top: 24,
                            left: '50%',
                            transform: 'translateX(-50%)',
                            zIndex: 1100,
                        }}
                    >
                        Return
                    </button>
                    <div style={{ width: '100vw', height: '100vh', display: 'flex', alignItems: 'center', justifyContent: 'center' }}>
                        <InteractiveScheme resetLayout={resetLayout} />
                    </div>
                </div>
            )}
        </>
    );
};

export default Home; 
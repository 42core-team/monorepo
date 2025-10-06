name: Bug Report
description: File a bug report
title: "[Bug] <short summary>"
labels: ["bug", "triage"]
assignees: []
projects: []
body:
  - type: markdown
    attributes:
      value: |
        Thanks for reporting a bug. Please fill in as much as possible.

  - type: input
    id: contact
    attributes:
      label: Contact
      description: How can we reach you for more info?
      placeholder: email@example.com

  - type: textarea
    id: summary
    attributes:
      label: What happened?
      description: What did you expect vs. what actually happened?
      placeholder: Clear, concise summary of the bug
    validations:
      required: true

  - type: textarea
    id: repro
    attributes:
      label: Steps to reproduce
      description: Step-by-step instructions to trigger the issue
      placeholder: |
        1. Go to ...
        2. Click ...
        3. Observe ...
      render: bash
    validations:
      required: true

  - type: dropdown
    id: severity
    attributes:
      label: Severity
      options:
        - S1 - Blocking (no workaround)
        - S2 - Major (workaround exists)
        - S3 - Minor (cosmetic or low impact)
      default: 1
    validations:
      required: true

  - type: dropdown
    id: component
    attributes:
      label: Affected component
      description: Select the most relevant area
      options:
        - Core Engine
        - Gameplay
        - UI
        - Networking
        - Build/CI
        - Docs
        - Other/Unknown
      default: 6
    validations:
      required: true

  - type: input
    id: version
    attributes:
      label: Version / branch / event / URL
      placeholder: e.g. v1.2.3, feature/xyz, event-2025-10-06, https://...
    validations:
      required: true

  - type: textarea
    id: env
    attributes:
      label: Environment
      description: OS, platform, runtime, hardware, etc.
      placeholder: e.g. macOS 14.5, Apple M3, Java 21, Node 20
    validations:
      required: true

  - type: checkboxes
    id: regression
    attributes:
      label: Regression?
      options:
        - label: This worked in a previous version
          required: false

  - type: textarea
    id: logs
    attributes:
      label: Relevant logs or stack traces
      description: Paste only the relevant bits
      render: shell

  - type: textarea
    id: attachments
    attributes:
      label: Screenshots / recordings
      description: Drag-and-drop images or attach links

  - type: checkboxes
    id: confirm
    attributes:
      label: Pre-flight checks
      options:
        - label: I searched existing issues
          required: true
        - label: I can reproduce this reliably
          required: true

  - type: textarea
    id: extra
    attributes:
      label: Additional context
      description: Anything else that might help
